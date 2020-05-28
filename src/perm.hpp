/**
 * @file
 *
 * Copyright 2020 Garmin Ltd. or its subsidiaries
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

#ifndef _PERM_HPP
#define _PERM_HPP

#include "config.h"

#include <memory>
#include <string>

struct stat;

namespace permfs {
extern const std::string config_file_name;

std::string const &root_path();
void warp_stats(std::string const &path, struct stat *s);

};  // namespace permfs

#endif /* _PERM_HPP */
