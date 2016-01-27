/*
 * Copyright (c) 2012-2015, Nic McDonald
 * All rights reserved.
 *
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
 * - Neither the name of prim nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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
#include "settings/Settings.h"

#include <cstdio>
#include <fstream>  // NOLINT
#include <sstream>

namespace settings {

void Settings::initFile(const char* _configFile, Json::Value* _settings) {
  // create an ifstream for the JSON reader
  std::ifstream is(_configFile, std::ifstream::binary);
  if (!is) {
    fprintf(stderr, "Settings error: could not open file '%s'\n", _configFile);
    exit(-1);
  }

  // read in the config file
  Json::Reader reader;
  bool success = reader.parse(is, *_settings, false);
  is.close();

  if (!success) {
    fprintf(stderr, "Settings error: failed to parse JSON file '%s'\n%s\n",
            _configFile, reader.getFormattedErrorMessages().c_str());
    exit(-1);
  }
}

void Settings::initString(const char* _config, Json::Value* _settings) {
  // read in the config file
  Json::Reader reader;
  bool success = reader.parse(_config, *_settings, false);

  if (!success) {
    fprintf(stderr, "Settings error: failed to parse JSON string:\n%s\n%s",
            _config, reader.getFormattedErrorMessages().c_str());
    exit(-1);
  }
}

std::string Settings::toString(const Json::Value& _settings) {
  Json::StyledWriter writer;
  std::stringstream ss;
  ss << writer.write(_settings);
  return ss.str();
}

void Settings::update(Json::Value* _settings,
                      const std::vector<std::string>& _updates) {
  for (auto it = _updates.cbegin(); it != _updates.cend(); ++it) {
    const std::string& overwrite = *it;

    size_t equalsLoc = overwrite.find_first_of('=');
    size_t atSymLoc = overwrite.find_last_of('=');
    if ((equalsLoc == std::string::npos) ||
        (atSymLoc == std::string::npos) ||
        (atSymLoc <= equalsLoc + 1)) {
      fprintf(stderr, "invalid setting overwrite spec: %s\n",
              overwrite.c_str());
      exit(-1);
    }

    std::string pathStr = overwrite.substr(0, equalsLoc);
    std::string varType = overwrite.substr(equalsLoc + 1,
                                           atSymLoc - equalsLoc - 1);
    std::string valueStr = overwrite.substr(atSymLoc + 1);

    Json::Path path(pathStr);
    Json::Value& setting = path.make(*_settings);
    if (varType == "int") {
      setting = Json::Value(std::stoll(valueStr));
    } else if (varType == "uint") {
      setting = Json::Value(std::stoull(valueStr));
    } else if (varType == "float") {
      setting = Json::Value(std::stod(valueStr));
    } else if (varType == "string") {
      setting = Json::Value(valueStr);
    } else if (varType == "bool") {
      if (valueStr == "true" || valueStr == "1") {
        setting = Json::Value(true);
      } else if (valueStr == "false" || valueStr == "0") {
        setting = Json::Value(false);
      } else {
        fprintf(stderr, "invalid bool: %s\n", valueStr.c_str());
        exit(-1);
      }
    } else {
      fprintf(stderr, "invalid setting type: %s\n", varType.c_str());
      exit(-1);
    }
  }
}

void Settings::commandLine(s32 _argc, const char* const* _argv,
                           Json::Value* _settings) {
  // create a settings object
  if (_argc < 2) {
    fprintf(stderr, "Please specify a settings file\n");
    exit(-1);
  }
  const char* settingsFile = _argv[1];
  Settings::initFile(settingsFile, _settings);

  // read in settings overwrites
  std::vector<std::string> settingsUpdates;
  for (s64 arg = 2; arg < _argc; arg++) {
    settingsUpdates.push_back(std::string(_argv[arg]));
  }
  update(_settings, settingsUpdates);
}

}  // namespace settings
