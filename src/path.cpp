/**
 * @file
 *
 * Copyright 2020 by Garmin Ltd. or its subsidiaries.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "path.hpp"

#include <libgen.h>

#include <cstring>
#include <memory>
#include <string>

#include "perm.hpp"

/*
 * glibc defines basename as a macro. In order to allow us to define a function
 * also named basename with out it being expanded to a different name, wrap the
 * C library basename function in a helper and then undefine the basename macro
 */
static char *basename_helper(char *s) { return ::basename(s); }

#undef basename

std::string permfs::path::basename(std::string const &path) {
  char *b = strdup(path.c_str());
  std::string s = basename_helper(b);
  free(b);
  return s;
}

std::string permfs::path::dirname(std::string const &path) {
  char *b = strdup(path.c_str());
  std::string s = ::dirname(b);
  return s;
}

std::unique_ptr<std::string> permfs::path::convert_path(char const *path) {
  if (permfs::path::basename(path) == permfs::config_file_name) {
    return nullptr;
  }
  return std::make_unique<std::string>(permfs::root_path() + "/" + path);
}
