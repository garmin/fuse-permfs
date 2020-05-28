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

#include "config.h"

#include "fs.hpp"

#include <errno.h>
#include <fuse.h>
#include <unistd.h>

#include "path.hpp"
#include "perm.hpp"

static int perm_getattr(char const *path, struct stat *s,
                        struct fuse_file_info *info) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  if (::lstat(p->c_str(), s) < 0) {
    return -errno;
  }
  permfs::warp_stats(path, s);
  return 0;
}

static int perm_readlink(char const *path, char *buf, size_t size) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  auto res = ::readlink(p->c_str(), buf, size - 1);

  if (res < 0) {
    return -errno;
  }
  buf[res] = '\0';
  return 0;
}

static int perm_mknod(char const *path, mode_t mode, dev_t rdev) {
  return -EROFS;
}

static int perm_mkdir(char const *path, mode_t mode) { return -EROFS; }

static int perm_unlink(char const *path) { return -EROFS; }

static int perm_rmdir(char const *path) { return -EROFS; }

static int perm_symlink(char const *from, char const *to) { return -EROFS; }

static int perm_rename(char const *from, char const *to, unsigned int flags) {
  return -EROFS;
}

static int perm_link(char const *from, char const *to) { return -EROFS; }

static int perm_chmod(char const *path, mode_t mode,
                      struct fuse_file_info *info) {
  return -EROFS;
}

static int perm_chown(char const *path, uid_t uid, gid_t gid,
                      struct fuse_file_info *info) {
  return -EROFS;
}

static int perm_truncate(char const *path, off_t size,
                         struct fuse_file_info *info) {
  return -EROFS;
}

static int perm_statfs(char const *path, struct statvfs *stat) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  if (::statvfs(p->c_str(), stat) < 0) {
    return -errno;
  }
  return 0;
}

static int perm_access(char const *path, int mode) {
  auto const p = permfs::path::convert_path(path);
  if (!p) return -ENOENT;

  if (mode & W_OK) {
    return -EROFS;
  }

  if (::access(p->c_str(), mode) < 0) {
    return -errno;
  }

  return 0;
}

static int perm_utimens(char const *path, struct timespec const tv[2],
                        struct fuse_file_info *info) {
  return -EROFS;
}

void permfs::fs::configure_operations(struct fuse_operations *ops) {
  ops->getattr = perm_getattr;
  ops->readlink = perm_readlink;
  ops->mknod = perm_mknod;
  ops->mkdir = perm_mkdir;
  ops->unlink = perm_unlink;
  ops->rmdir = perm_rmdir;
  ops->symlink = perm_symlink;
  ops->rename = perm_rename;
  ops->link = perm_link;
  ops->chmod = perm_chmod;
  ops->chown = perm_chown;
  ops->truncate = perm_truncate;
  ops->statfs = perm_statfs;
  ops->access = perm_access;
  ops->utimens = perm_utimens;
}
