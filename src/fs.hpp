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

#ifndef _FS_HPP
#define _FS_HPP

#include "config.h"

struct fuse_operations;

namespace permfs {
namespace fs {

void configure_operations(struct fuse_operations *ops);

}  // namespace fs
}  // namespace permfs

#endif /* _FS_HPP */
