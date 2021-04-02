/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * - Neither the name of prim nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "settings/settings.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>  // NOLINT
#include <iomanip>
#include <memory>
#include <queue>
#include <sstream>
#include <stack>
#include <unordered_set>

#include "fio/InFile.h"
#include "fio/OutFile.h"
#include "strop/strop.h"

namespace settings {

// This defines the maximum amount of file inclusion depth allowed.
// This blocks against infinite recursion.
static const u32 MAX_INCLUSION_DEPTH = 100;

// Prints the usage ("-h" or "--help") message.
static void usage(const char* _exe, const char* _error);

// Utilities for filesystem path parsing.
static std::string dirname(const std::string& _path);
static std::string join(const std::string& _a, const std::string& _b);

// Loads the JSON::Value represented in the file.
// Recursively performs file inclusion.
// Error prints and exit(-1) upon failure.
static void fileToJson(const std::string& _config, nlohmann::json* _settings,
                       u32 _recursion_depth);

// Loads the JSON::Value represented by the string.
// Recursively performs file inclusion.
// Error prints and exit(-1) upon failure.
static void stringToJson(const std::string& _config, nlohmann::json* _settings,
                         const std::string& _filename, const std::string& _cwd,
                         u32 _recursion_depth);

// This replaces "$$(...)$$" references with file JSON contents.
static void processInclusions(const std::string& _cwd,
                              nlohmann::json* _settings, u32 _recursion_depth);

// This replaces "$&(...)&$" reference with nlohmann::json contents.
static void processReferences(nlohmann::json* _settings);

// This applies command line updates to the current settings.
// This will perform inclusions but not references.
static void applyUpdates(nlohmann::json* _settings,
                         const std::vector<std::string>& _updates, bool _debug);

// This is a debug printer utility for printing debug info.
static void dprintf(bool _debug, const char* _format, ...);

/*** public functions below here ***/

void initFile(const std::string& _config_file, nlohmann::json* _settings) {
  // Parses the file into JSON.
  fileToJson(_config_file, _settings, 1);
  // Process all references.
  processReferences(_settings);
}

void initString(const std::string& _config_str, nlohmann::json* _settings) {
  // Parses the string into JSON.
  stringToJson(_config_str, _settings, "", ".", 1);
  // Process all references.
  processReferences(_settings);
}

void commandLine(s32 _argc, const char* const* _argv,
                 nlohmann::json* _settings) {
  assert(_argc > 0);

  // Scan for:
  //  -h or --help
  //  -d or --debug
  bool debug = false;
  s32 first = 1;
  for (s32 i = first; i < _argc; i++) {
    // Checks for leading '-'.
    if (_argv[i][0] == '-') {
      // Checks for help.
      if ((strcmp(_argv[i], "-h") == 0) || (strcmp(_argv[i], "--help") == 0)) {
        usage(_argv[0], nullptr);
        exit(0);
      } else if ((strcmp(_argv[i], "-d") == 0) ||
                 (strcmp(_argv[i], "--debug") == 0)) {
        debug = true;
      }
      first++;
    } else {
      break;
    }
  }
  dprintf(debug, "first non-flag location is %i\n", first);

  // Creates a settings object.
  if (_argc <= first) {
    usage(_argv[0], "Please specify a settings file\n");
    exit(-1);
  }
  std::string config_file = _argv[first];

  // Parses the file into JSON.
  dprintf(debug, "beginning parsing of JSON file %s\n", config_file.c_str());
  fileToJson(config_file, _settings, 1);
  dprintf(debug, "parsing of JSON file %s complete\n", config_file.c_str());

  // Reads in settings updates.
  std::vector<std::string> settings_updates;
  for (s64 arg = first + 1; arg < _argc; arg++) {
    std::string update(_argv[arg]);
    dprintf(debug, "adding update: %s\n", update.c_str());
    settings_updates.push_back(update);
  }

  // Applies settings updates.
  applyUpdates(_settings, settings_updates, debug);

  // Processes. all references.
  processReferences(_settings);
}

std::string toString(const nlohmann::json& _settings) {
  std::ostringstream ss;
  ss << std::setw(2) << _settings << std::endl;
  return ss.str();
}

void writeToFile(const nlohmann::json& _settings,
                 const std::string& _config_file) {
  std::string text = toString(_settings);
  fio::OutFile::Status sts = fio::OutFile::writeFile(_config_file, text);
  if (sts != fio::OutFile::Status::OK) {
    fprintf(stderr, "Settings error: couldn't write to file %s\n",
            _config_file.c_str());
    exit(-1);
  }
}

/*** static functions below here ***/

static void usage(const char* _exe, const char* _error) {
  if (_error != nullptr) {
    printf("Settings error: %s\n", _error);
  }
  printf(
      "usage:\n"
      "  %s <file> [overrides] ...\n"
      "\n"
      "  file      : JSON formated settings file expressing configuration\n"
      "              (see examples)\n"
      "  override  : a descriptor of a settings override\n"
      "              <path_description>=<type>=<value>\n"
      "              type may be uint, float, string, bool, ref, file\n"
      "              path descriptors follow RFC 6901\n"
      "\n"
      "              ### simple examples ###\n"
      "              /this/is/a/deep/path=uint=1200\n"
      "              /important/values/3=float=10.89\n"
      "              /stats/logfile/compress=bool=false\n"
      "\n"
      "              ### complex examples ###\n"
      "              /some/setting=ref=/some/other/setting\n"
      "              /my_array=int=[1,6,4,8,999]\n"
      "              /elsewhere/settings=file=\"somedir/somefile.json\"\n"
      "\n"
      "              ### really complex examples ###\n"
      "              /me=file=[a.json,b.json,c.json]\n"
      "              /you=ref=[/me/2,/me/0,/me/1]\n"
      "\n",
      _exe);
}

static std::string dirname(const std::string& _path) {
  size_t idx = _path.find_last_of('/');
  if (idx == std::string::npos) {
    return ".";
  } else {
    return _path.substr(0, idx + 1);
  }
}

static std::string join(const std::string& _a, const std::string& _b) {
  if (_a.at(_a.size() - 1) != '/') {
    return _a + '/' + _b;
  } else {
    return _a + _b;
  }
}

static void fileToJson(const std::string& _config, nlohmann::json* _settings,
                       u32 _recursion_depth) {
  assert(_recursion_depth <= MAX_INCLUSION_DEPTH);
  if (_recursion_depth == MAX_INCLUSION_DEPTH) {
    fprintf(stderr,
            "Settings error: max inclusion depth reached\n"
            "You likely have an infinite file inclusion cycle\n");
    exit(-1);
  }

  std::string dir = dirname(_config);

  // Reads the file into a string.
  std::string text;
  fio::InFile::Status sts = fio::InFile::readFile(_config, &text);
  assert(sts == fio::InFile::Status::OK);

  // Parses the string into JSON.
  stringToJson(text, _settings, _config, dir, _recursion_depth);
}

static void stringToJson(const std::string& _config, nlohmann::json* _settings,
                         const std::string& _filename, const std::string& _cwd,
                         u32 _recursion_depth) {
  // Parses the JSON string.
  try {
    *(_settings) = nlohmann::json::parse(_config);
  } catch (nlohmann::json::parse_error& e) {
    if (_filename == "") {
      fprintf(stderr, "Settings error: failed to parse JSON file:%s\n%s",
              _filename.c_str(), e.what());
    } else {
      fprintf(stderr, "Settings error: failed to parse JSON string:\n%s\n%s",
              _config.c_str(), e.what());
    }
    exit(-1);
  }

  // Performs JSON inclusions.
  processInclusions(_cwd, _settings, _recursion_depth);
}

static void processInclusions(const std::string& _cwd,
                              nlohmann::json* _settings, u32 _recursion_depth) {
  // Performs inclusion processing via BFS.
  std::queue<nlohmann::json*> queue;
  queue.push(_settings);

  while (!queue.empty()) {
    nlohmann::json* parent = queue.front();
    queue.pop();
    for (auto& item : parent->items()) {
      nlohmann::json& child = item.value();

      // Checks if an insertion is needed.
      if (child.is_string()) {
        std::string chstr = child.get<std::string>();
        if ((chstr.size() > 6) && (chstr.substr(0, 3) == "$$(") &&
            (chstr.substr(chstr.size() - 3, 3) == ")$$")) {
          // Extracts the subsettings filepath.
          std::string filepath = chstr.substr(3, chstr.size() - 6);

          // Parses the subsettings.
          nlohmann::json subsettings;
          fileToJson(join(_cwd, filepath), &subsettings, _recursion_depth + 1);

          // Performs insertion.
          child = subsettings;
        }
      }

      // Adds item to BFS queue.
      if (&child != parent) {
        queue.push(&child);
      }
    }
  }
}

static void processReferences(nlohmann::json* _settings) {
  // Performs reference processing via BFS.
  std::queue<nlohmann::json*> queue;
  queue.push(_settings);

  while (!queue.empty()) {
    nlohmann::json* parent = queue.front();
    queue.pop();
    for (auto& item : parent->items()) {
      nlohmann::json& child = item.value();

      // Checks if an insertion is needed.
      if (child.is_string()) {
        std::string chstr = child.get<std::string>();
        if ((chstr.size() > 6) && (chstr.substr(0, 3) == "$&(") &&
            (chstr.substr(chstr.size() - 3, 3) == ")&$")) {
          // Extracts the settings path.
          std::string path_str = chstr.substr(3, chstr.size() - 6);
          nlohmann::json::json_pointer ptr;
          try {
            ptr = nlohmann::json::json_pointer(path_str);
          } catch (nlohmann::json::parse_error& e) {
            printf("Pointer specification \"%s\" caused a failure\n:%s\n",
                   path_str.c_str(), e.what());
            exit(-1);
          }

          // Performs insertion.
          child = (*_settings)[ptr];
        }
      }

      // Adds item to BFS queue.
      if (&child != parent) {
        queue.push(&child);
      }
    }
  }
}

static void applyUpdates(nlohmann::json* _settings,
                         const std::vector<std::string>& _updates,
                         bool _debug) {
  for (auto it = _updates.cbegin(); it != _updates.cend(); ++it) {
    // Gets the update string.
    const std::string& update = *it;
    dprintf(_debug, "applying update: %s\n", update.c_str());

    // Splits the update string into symbols.
    size_t equalsLoc = update.find_first_of('=');
    size_t atSymLoc = update.find_last_of('=');
    if ((equalsLoc == std::string::npos) || (atSymLoc == std::string::npos) ||
        (atSymLoc <= equalsLoc + 1)) {
      fprintf(stderr, "Settings error: invalid setting update spec: %s\n",
              update.c_str());
      exit(-1);
    }

    std::string path_str = update.substr(0, equalsLoc);
    std::string var_type =
        update.substr(equalsLoc + 1, atSymLoc - equalsLoc - 1);
    std::string value_str = update.substr(atSymLoc + 1);

    // Determines if the value is an array type.
    bool is_array = ((value_str.at(0) == '[') &&
                     (value_str.at(value_str.size() - 1) == ']'));
    std::vector<std::string> value_elems;
    if (!is_array) {
      // Puts the full value in the array.
      value_elems.push_back(value_str);
    } else {
      // Removes the [] if this is an array.
      value_str = value_str.substr(1, value_str.size() - 2);
      value_elems = strop::split(value_str, ',');
    }

    // Converts all strings to a nlohmann::json array.
    std::vector<nlohmann::json> array(value_elems.size());
    for (u32 idx = 0; idx < value_elems.size(); idx++) {
      if (var_type == "int") {
        const s64 val = std::stoll(value_elems[idx]);
        array[idx] = val;
      } else if (var_type == "uint") {
        const u64 val = std::stoull(value_elems[idx]);
        array[idx] = val;
      } else if (var_type == "float") {
        const f64 val = std::stod(value_elems[idx]);
        array[idx] = val;
      } else if (var_type == "string") {
        array[idx] = value_elems[idx];
      } else if (var_type == "bool") {
        if (value_elems[idx] == "true" || value_elems[idx] == "1") {
          array[idx] = true;
        } else if (value_elems[idx] == "false" || value_elems[idx] == "0") {
          array[idx] = false;
        } else {
          fprintf(stderr, "Settings error: invalid bool: %s\n",
                  value_elems[idx].c_str());
          exit(-1);
        }
      } else if (var_type == "file") {
        nlohmann::json subsettings;
        fileToJson(value_elems[idx], &subsettings, 2);
        array[idx] = subsettings;
      } else if (var_type == "ref") {
        // Just fake it as a string for now.
        array[idx] = "$&(" + value_elems[idx] + ")&$";
      } else {
        fprintf(stderr, "Settings error: invalid setting type: %s\n",
                var_type.c_str());
        exit(-1);
      }
    }

    // Uses the path to find the location.
    nlohmann::json::json_pointer ptr;
    try {
      ptr = nlohmann::json::json_pointer(path_str);
    } catch (nlohmann::json::parse_error& e) {
      printf("Pointer specification \"%s\" caused a failure\n:%s\n",
             path_str.c_str(), e.what());
      exit(-1);
    }

    // Makes the update.
    if (!is_array) {
      assert(array.size() == 1u);
      (*_settings)[ptr] = array[0];
    } else {
      (*_settings)[ptr] = array;
    }
  }
}

void dprintf(bool _debug, const char* _format, ...) {
  if (_debug) {
    printf("Settings debug: ");
    va_list args;
    va_start(args, _format);
    vprintf(_format, args);
    va_end(args);
  }
}

}  // namespace settings
