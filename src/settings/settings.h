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
#ifndef SETTINGS_SETTINGS_H_
#define SETTINGS_SETTINGS_H_

#include <nlohmann/json.hpp>
#include <prim/prim.h>

#include <string>
#include <vector>

namespace settings {

// this initializes the settings from a JSON file
//  error print and exit(-1) upon failure
void initFile(const std::string& _configFile, nlohmann::json* _settings);

// this initializes the settings from a JSON string
//  error print and exit(-1) upon failure
void initString(const std::string& _configStr, nlohmann::json* _settings);

// this initializes the settings from a JSON file and settings updates
//  pass the "-h" flag to see how to use
//  error print and exit(-1) upon failure
void commandLine(s32 _argc, const char* const* _argv,
                 nlohmann::json* _settings);

// this returns a string representation of the settings
std::string toString(const nlohmann::json& _settings);

// writes settings to a file
void writeToFile(const nlohmann::json& _settings,
                 const std::string& _configFile);

}  // namespace settings

#endif  // SETTINGS_SETTINGS_H_
