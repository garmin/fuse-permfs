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

#include "config.h"

#include "file.hpp"

#include <errno.h>
#include <fuse.h>
#include <unistd.h>

#include "path.hpp"
#include "perm.hpp"

static int file_open(char const *path, struct fuse_file_info *info) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  if (info->flags & (O_CREAT | O_RDWR | O_WRONLY)) {
    return -EROFS;
  }

  int fd = ::open(p->c_str(), info->flags);
  if (fd < 0) {
    return -errno;
  }

  info->fh = fd;

  return 0;
}

static int file_read(char const *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *info) {
  ssize_t res = ::pread(info->fh, buf, size, offset);
  if (res < 0) {
    return -errno;
  }
  return res;
}

static int file_write(char const *path, char const *buf, size_t size,
                      off_t offset, struct fuse_file_info *info) {
  return -EROFS;
}

static int file_release(char const *path, struct fuse_file_info *info) {
  ::close(info->fh);
  return 0;
}

void permfs::file::configure_operations(struct fuse_operations *ops) {
  ops->open = file_open;
  ops->read = file_read;
  ops->write = file_write;
  ops->release = file_release;
}
