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

#include <fnmatch.h>
#include <sys/stat.h>
#include <yaml-cpp/yaml.h>

#include <iostream>

#include "path.hpp"
#include "perm.hpp"

static void replace_mode(mode_t *mode, mode_t new_mode) {
  *mode = (*mode & ~0777) | (new_mode & 0777);
}

static void apply_perm(YAML::Node const &node, struct stat *s) {
  if (node["uid"]) {
    s->st_uid = node["uid"].as<int>();
  }

  if (node["gid"]) {
    s->st_gid = node["gid"].as<int>();
  }

  if (S_ISDIR(s->st_mode)) {
    if (node["dmode"]) {
      replace_mode(&s->st_mode, node["dmode"].as<mode_t>());
    }
  } else {
    if (node["fmode"]) {
      replace_mode(&s->st_mode, node["fmode"].as<mode_t>());
    }
  }
}

void permfs::warp_stats(std::string const &path, struct stat *s) {
  auto filename = permfs::path::basename(path);
  auto parent = permfs::path::dirname(path);

  // If no other match is found, assign the default permissions
  s->st_uid = 0;
  s->st_gid = 0;
  if (S_ISDIR(s->st_mode)) {
    replace_mode(&s->st_mode, 0755);
  } else {
    replace_mode(&s->st_mode, 0644);
  }

  std::list<std::pair<std::string, std::shared_ptr<YAML::Node> > > nodes;

  auto match = filename;
  while (true) {
    std::string config_file =
        permfs::root_path() + "/" + parent + "/" + config_file_name;

    std::cout << "Checking " << config_file << " " << parent << " " << filename
              << std::endl;

    try {
      auto node = std::make_shared<YAML::Node>(YAML::LoadFile(config_file));
      std::cout << "Parsed " << config_file << std::endl;

      nodes.push_front(std::make_pair(match, node));

    } catch (YAML::BadConversion &e) {
      std::cerr << "Conversion error in " << config_file << ": " << e.what()
                << std::endl;
    } catch (YAML::ParserException &e) {
      std::cerr << "Error parsing " << config_file << ": " << e.what()
                << std::endl;
    } catch (YAML::BadFile &e) {
      // std::cout << "Cannot open " << config_file << ": " << e.what() <<
      // std::endl;
    }

    if (match == "/" && parent == "/") {
      break;
    }

    match = permfs::path::basename(parent);
    parent = permfs::path::dirname(parent);
  }

  for (auto const &l : nodes) {
    auto const &match = l.first;
    auto const node = l.second;
    std::cout << l.first << std::endl;
    if (match == "/" && parent == "/") {
      // The real root doesn't match a glob in it's own config
      if ((*node)["/"]) {
        apply_perm((*node)["/"], s);
      }
    } else if ((*node)["children"]) {
      auto const cnode = (*node)["children"];
      for (auto i = cnode.begin(); i != cnode.end(); ++i) {
        auto const pattern = i->first.as<std::string>();
        std::cout << "Checking " << match << " against " << pattern;
        if (fnmatch(pattern.c_str(), match.c_str(), 0) == 0) {
          std::cout << " matched!" << std::endl;
          apply_perm(i->second, s);
        } else {
          std::cout << " no match" << std::endl;
        }
      }
    }
  }
}
