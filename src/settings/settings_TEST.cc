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
#include <json/json.h>
#include <gtest/gtest.h>

#include <string>

#include "settings/settings.h"

const char* JSON =
    "{\n"
    "   \"age\" : 30,\n"
    "   \"family\" : {\n"
    "      \"kids\" : [\n"
    "         {\n"
    "            \"age\" : 3,\n"
    "            \"name\" : \"Kaylee\"\n"
    "         },\n"
    "         {\n"
    "            \"age\" : 0,\n"
    "            \"name\" : \"Ruby\"\n"
    "         }\n"
    "      ],\n"
    "      \"wife\" : {\n"
    "         \"age\" : 27,\n"
    "         \"name\" : \"Kara\"\n"
    "      }\n"
    "   },\n"
    "   \"name\" : \"Nic\"\n"
    "}\n";

void Settings_TEST(const Json::Value& settings) {
  bool debug = false;
  if (debug) {
    printf("%s\n", settings::toString(settings).c_str());
  }

  ASSERT_EQ(settings.size(), 3u);

  Json::Value name = settings["name"];
  ASSERT_EQ(name.size(), 0u);
  ASSERT_EQ(name.asString(), "Nic");

  Json::Value age = settings["age"];
  ASSERT_EQ(age.size(), 0u);
  ASSERT_EQ(age.asUInt(), 30u);

  Json::Value family = settings["family"];
  ASSERT_EQ(family.size(), 2u);

  Json::Value wife = family["wife"];
  ASSERT_EQ(wife.size(), 2u);
  ASSERT_EQ(wife["name"].asString(), "Kara");
  ASSERT_EQ(wife["age"].asUInt(), 27u);

  Json::Value kids = family["kids"];
  ASSERT_EQ(kids.size(), 2u);

  Json::Value kid0 = kids[0];
  ASSERT_EQ(kid0.size(), 2u);
  ASSERT_EQ(kid0["name"].asString(), "Kaylee");
  ASSERT_EQ(kid0["age"].asUInt(), 3u);

  Json::Value kid1 = kids[1];
  ASSERT_EQ(kid1.size(), 2u);
  ASSERT_EQ(kid1["name"].asString(), "Ruby");
  ASSERT_EQ(kid1["age"].asUInt(), 0u);
}

TEST(Settings, string) {
  Json::Value settings;
  settings::initString(JSON, &settings);

  Settings_TEST(settings);
}

TEST(Settings, file) {
  const char* filename = "TEST_settings.json";
  FILE* fp = fopen(filename, "w");
  assert(fp != NULL);
  fprintf(fp, "%s", JSON);
  fclose(fp);

  Json::Value settings;
  settings::initFile(filename, &settings);

  Settings_TEST(settings);

  assert(remove(filename) == 0);
}

TEST(Settings, toString) {
  Json::Value settings;
  settings::initString(JSON, &settings);
  std::string jsonStr = settings::toString(settings);
  // printf("%s'''\n", JSON);
  // printf("%s'''\n", jsonStr.c_str());
  ASSERT_EQ(jsonStr, std::string(JSON));
}

TEST(Settings, commandLine1) {
  const char* filename = "TEST_settings.json";
  FILE* fp = fopen(filename, "w");
  assert(fp != NULL);
  fprintf(fp, "%s", JSON);
  fclose(fp);

  const int argc = 4;
  const char* argv[argc] = {
    "./path/to/some/binary",
    "TEST_settings.json",
    "family.kids[0].name=string=Krazy",
    "family.wife.sexy=bool=true"
  };

  Json::Value settings;
  settings::commandLine(argc, argv, &settings);

  // override #1
  Json::Value kid0 = settings["family"]["kids"][0];
  ASSERT_EQ(kid0.size(), 2u);
  ASSERT_EQ(kid0["name"].asString(), "Krazy");
  ASSERT_EQ(kid0["age"].asUInt(), 3u);

  // override #2
  Json::Value wife = settings["family"]["wife"];
  ASSERT_EQ(wife.size(), 3u);
  ASSERT_EQ(wife["name"].asString(), "Kara");
  ASSERT_EQ(wife["age"].asUInt(), 27u);
  ASSERT_TRUE(wife.isMember("sexy"));
  ASSERT_EQ(wife["sexy"].asBool(), true);
}

TEST(Settings, commandLine2) {
  const char* filename = "TEST_settings.json";
  FILE* fp = fopen(filename, "w");
  assert(fp != NULL);
  fprintf(fp, "%s", JSON);
  fclose(fp);

  const int argc = 2;
  const char* argv[argc] = {
    "./path/to/some/binary",
    "TEST_settings.json"
  };

  Json::Value settings;
  settings::commandLine(argc, argv, &settings);

  Settings_TEST(settings);

  assert(remove(filename) == 0);
}

TEST(Settings, commandLine3) {
  const char* filename = "TEST_settings.json";
  FILE* fp = fopen(filename, "w");
  assert(fp != NULL);
  fprintf(fp, "%s", JSON);
  fclose(fp);

  const int argc = 5;
  const char* argv[argc] = {
    "./path/to/some/binary",
    "TEST_settings.json",
    "age=string=veryold",
    "family.kids[1].name=string=Tuby",
    "family.wife.age=int=-10"
  };

  Json::Value settings;
  settings::commandLine(argc, argv, &settings);

  ASSERT_EQ(settings["age"].asString(), "veryold");
  ASSERT_EQ(settings["family"]["kids"][1]["name"].asString(), "Tuby");
  ASSERT_EQ(settings["family"]["wife"]["age"].asInt(), -10);

  assert(remove(filename) == 0);
}

TEST(Settings, subsettings_initFile) {
  const char* afilename = "TEST_asettings.json";
  FILE* afp = fopen(afilename, "w");
  assert(afp != NULL);
  fprintf(afp, "%s", "{\"sub\": \"$$(TEST_bsettings.json)$$\", \"a\": 1}");
  fclose(afp);

  const char* bfilename = "TEST_bsettings.json";
  FILE* bfp = fopen(bfilename, "w");
  assert(bfp != NULL);
  fprintf(bfp, "%s", "[\"b\", false, \"$$(TEST_csettings.json)$$\", \"b\", 1]");
  fclose(bfp);

  const char* cfilename = "TEST_csettings.json";
  FILE* cfp = fopen(cfilename, "w");
  assert(cfp != NULL);
  fprintf(cfp, "%s", "{\"x\":{\"y\":{\"z\":\"$$(TEST_dsettings.json)$$\"}}}");
  fclose(cfp);

  const char* dfilename = "TEST_dsettings.json";
  FILE* dfp = fopen(dfilename, "w");
  assert(dfp != NULL);
  fprintf(dfp, "%s", "12345678");
  fclose(dfp);

  Json::Value settings;
  settings::initFile(afilename, &settings);

  ASSERT_EQ(settings["sub"][2]["x"]["y"]["z"].asUInt(), 12345678u);

  assert(remove(afilename) == 0);
  assert(remove(bfilename) == 0);
  assert(remove(cfilename) == 0);
  assert(remove(dfilename) == 0);
}

TEST(Settings, subsettings_initString) {
  const char* afilename = "TEST_asettings.json";
  FILE* afp = fopen(afilename, "w");
  assert(afp != NULL);
  fprintf(afp, "%s", "{\"sub\": \"$$(TEST_bsettings.json)$$\", \"a\": 1}");
  fclose(afp);

  const char* bfilename = "TEST_bsettings.json";
  FILE* bfp = fopen(bfilename, "w");
  assert(bfp != NULL);
  fprintf(bfp, "%s", "[\"b\", false, \"$$(TEST_csettings.json)$$\", \"b\", 1]");
  fclose(bfp);

  const char* cfilename = "TEST_csettings.json";
  FILE* cfp = fopen(cfilename, "w");
  assert(cfp != NULL);
  fprintf(cfp, "%s", "{\"x\":{\"y\":{\"z\":\"$$(TEST_dsettings.json)$$\"}}}");
  fclose(cfp);

  const char* dfilename = "TEST_dsettings.json";
  FILE* dfp = fopen(dfilename, "w");
  assert(dfp != NULL);
  fprintf(dfp, "%s", "12345678");
  fclose(dfp);

  Json::Value settings;
  settings::initString("{\"top\": \"$$(TEST_asettings.json)$$\"}", &settings);

  ASSERT_EQ(settings["top"]["sub"][2]["x"]["y"]["z"].asUInt(), 12345678u);

  assert(remove(afilename) == 0);
  assert(remove(bfilename) == 0);
  assert(remove(cfilename) == 0);
  assert(remove(dfilename) == 0);
}

TEST(Settings, subsettings_commandLine) {
  const char* afilename = "TEST_asettings.json";
  FILE* afp = fopen(afilename, "w");
  assert(afp != NULL);
  fprintf(afp, "%s", "{\"sub\": \"$$(TEST_bsettings.json)$$\", \"a\": 1}");
  fclose(afp);

  const char* bfilename = "TEST_bsettings.json";
  FILE* bfp = fopen(bfilename, "w");
  assert(bfp != NULL);
  fprintf(bfp, "%s", "[\"b\", false, \"$$(TEST_csettings.json)$$\", \"b\", 1]");
  fclose(bfp);

  const char* cfilename = "TEST_csettings.json";
  FILE* cfp = fopen(cfilename, "w");
  assert(cfp != NULL);
  fprintf(cfp, "%s", "{\"x\":{\"y\":{\"z\":\"$$(TEST_dsettings.json)$$\"}}}");
  fclose(cfp);

  const char* dfilename = "TEST_dsettings.json";
  FILE* dfp = fopen(dfilename, "w");
  assert(dfp != NULL);
  fprintf(dfp, "%s", "12345678");
  fclose(dfp);

  const char* efilename = "TEST_esettings.json";
  FILE* efp = fopen(efilename, "w");
  assert(efp != NULL);
  fprintf(efp, "%s", "{\"n\": \"$$(TEST_fsettings.json)$$\"}");
  fclose(efp);

  const char* ffilename = "TEST_fsettings.json";
  FILE* ffp = fopen(ffilename, "w");
  assert(ffp != NULL);
  fprintf(ffp, "%s", "3.14159265359");
  fclose(ffp);

  const int argc = 4;
  const char* argv[argc] = {
    "./path/to/some/binary",
    "TEST_asettings.json",
    "toplevel=string=wahoo",
    "sub[2].x.y.m=file=TEST_esettings.json"
  };

  Json::Value settings;
  settings::commandLine(argc, argv, &settings);

  ASSERT_EQ(settings["sub"][2]["x"]["y"]["z"].asUInt(), 12345678u);
  ASSERT_EQ(settings["toplevel"].asString(), "wahoo");
  ASSERT_EQ(settings["sub"][2]["x"]["y"]["m"]["n"].asDouble(), 3.14159265359);

  assert(remove(afilename) == 0);
  assert(remove(bfilename) == 0);
  assert(remove(cfilename) == 0);
  assert(remove(dfilename) == 0);
  assert(remove(efilename) == 0);
  assert(remove(ffilename) == 0);
}
