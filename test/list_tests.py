#! /usr/bin/env python3
#
# Copyright 2020 Garmin Ltd. or its subsidiaries
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
import os
import sys

THIS_DIR = os.path.dirname(__file__)


def main():
    def print_test(suite):
        try:
            for s in suite:
                print_test(s)
        except TypeError:
            print(suite.id())

    print_test(unittest.defaultTestLoader.discover(THIS_DIR))


if __name__ == "__main__":
    sys.exit(main())
