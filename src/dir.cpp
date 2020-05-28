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

#include "dir.hpp"

#include <dirent.h>
#include <fuse.h>

#include <cstring>
#include <memory>

#include "path.hpp"
#include "perm.hpp"

struct perm_dir {
  DIR *d;
  struct dirent *entry;
  off_t offset;
};

static int dir_opendir(char const *path, struct fuse_file_info *info) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  auto d = std::make_unique<struct perm_dir>();

  d->d = ::opendir(p->c_str());
  if (!d->d) {
    return -errno;
  }
  d->offset = 0;
  d->entry = NULL;

  info->fh = reinterpret_cast<uint64_t>(d.release());

  return 0;
}

static int dir_readdir(char const *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *info,
                       enum fuse_readdir_flags flags) {
  auto *d = reinterpret_cast<struct perm_dir *>(info->fh);

  if (offset != d->offset) {
    ::seekdir(d->d, offset - 1);
    d->entry = NULL;
    d->offset = offset;
  }

  while (true) {
    if (!d->entry) {
      /*
       * Read next entry
       */
      errno = 0;
      d->entry = ::readdir(d->d);
      if (!d->entry) {
        return -errno;
      }
    }

    off_t next = ::telldir(d->d);

    if (strcmp(d->entry->d_name, permfs::config_file_name.c_str()) != 0) {
      struct stat s;

      ::memset(&s, 0, sizeof(s));
      s.st_ino = d->entry->d_ino;
      s.st_mode = d->entry->d_type << 12;

      if (filler(buf, d->entry->d_name, &s, next,
                 static_cast<enum fuse_fill_dir_flags>(0))) {
        return 0;
      }
    }

    d->entry = NULL;
    d->offset = next;
  }
}

static int dir_releasedir(char const *path, struct fuse_file_info *info) {
  std::unique_ptr<struct perm_dir> d(
      reinterpret_cast<struct perm_dir *>(info->fh));
  closedir(d->d);
  return 0;
}

void permfs::dir::configure_operations(struct fuse_operations *ops) {
  ops->opendir = dir_opendir;
  ops->readdir = dir_readdir;
  ops->releasedir = dir_releasedir;
}
