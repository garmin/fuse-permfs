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

#include <fuse.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include "dir.hpp"
#include "file.hpp"
#include "fs.hpp"
#include "perm.hpp"

const std::string permfs::config_file_name = "perm.yml";

struct config {
  std::string root_path;
} my_config;

std::string const &permfs::root_path() { return my_config.root_path; }

static void *perm_init(struct fuse_conn_info *info,
                       struct fuse_config *config) {
  config->nullpath_ok = 1;
  return NULL;
}

static struct fuse_operations operations = {};

enum { KEY_HELP, KEY_VERSION };

static struct fuse_opt options[] = {
    // clang-format off
    FUSE_OPT_KEY("-h", KEY_HELP),
    FUSE_OPT_KEY("--help", KEY_HELP),
    FUSE_OPT_KEY("-V", KEY_VERSION),
    FUSE_OPT_KEY("--version", KEY_VERSION),
    FUSE_OPT_END
    // clang-format on
};

static int parse_options(void *data, char const *arg, int key,
                         struct fuse_args *outargs) {
  switch (key) {
    case FUSE_OPT_KEY_NONOPT:
      if (my_config.root_path.empty()) {
        char *p = realpath(arg, NULL);
        my_config.root_path = p;
        free(p);
        return 0;
      }
      return 1;
    case KEY_HELP:
      std::cerr << "usage: " << outargs->argv[0]
                << " [options] <source> <mountpoint>" << std::endl
                << std::endl;

      fuse_opt_add_arg(outargs, "-h");

      // Don't let FUSE also print a usage line
      outargs->argv[0][0] = '\0';

      fuse_main(outargs->argc, outargs->argv, &operations, NULL);
      ::exit(1);

    case KEY_VERSION:
      std::cerr << "permfs version " << VERSION << std::endl;
      fuse_opt_add_arg(outargs, "--version");
      fuse_main(outargs->argc, outargs->argv, &operations, NULL);
      ::exit(0);
  }

  return 1;
}

int main(int argc, char **argv) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  operations.init = perm_init;

  permfs::fs::configure_operations(&operations);
  permfs::file::configure_operations(&operations);
  permfs::dir::configure_operations(&operations);

  if (fuse_opt_parse(&args, &my_config, options, parse_options)) {
    std::cerr << "Invalid Arguments" << std::endl;
    return 1;
  }

  if (my_config.root_path.empty()) {
    std::cerr << "Source path missing!" << std::endl;
    return 1;
  }

  std::cout << "Root path is " << my_config.root_path << std::endl;

  umask(0);
  return fuse_main(args.argc, args.argv, &operations, NULL);
}
