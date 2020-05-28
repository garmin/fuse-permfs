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

import collections
import contextlib
import os
import signal
import stat
import subprocess
import tempfile
import time
import unittest

THIS_DIR = os.path.dirname(__file__)

SEARCH_PATHS = ["."] + os.environ.get("SEARCH_PATHS", "").split()

FileStat = collections.namedtuple("FileStat", ["uid", "gid", "mode"])


def list_paths(path, include_dirs=True):
    paths = []
    old_dir = os.path.abspath(os.getcwd())
    try:
        os.chdir(path)

        for dirpath, dirnames, filenames in os.walk("."):
            paths.extend(os.path.join(dirpath, f) for f in filenames)
            if include_dirs:
                paths.extend(os.path.join(dirpath, d) for d in dirnames)

        paths.sort()
    finally:
        os.chdir(old_dir)

    return paths


def get_stats(path, include_dirs=True):
    paths = list_paths(path, include_dirs=include_dirs)

    stats = {}
    for p in paths:
        s = os.lstat(os.path.join(path, p))
        stats[p] = FileStat(s.st_uid, s.st_gid, stat.filemode(s.st_mode))

    if include_dirs:
        s = os.lstat(path)
        stats["."] = FileStat(s.st_uid, s.st_gid, stat.filemode(s.st_mode))

    return stats


class TestPermFS(unittest.TestCase):
    maxDiff = 10000

    def setUp(self):
        self.permfs = None

        for path in SEARCH_PATHS:
            p = os.path.join(path, "permfs")
            if os.path.exists(p):
                self.permfs = os.path.abspath(p)
                break

        self.assertIsNotNone(
            self.permfs, "permfs not found. Searched %s" % (" ".join(SEARCH_PATHS))
        )

    def assertSubprocess(self, *args, returncode=0, **kwargs):
        p = subprocess.run(
            *args, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, **kwargs
        )
        self.assertEqual(p.returncode, returncode, msg="%s failed" % " ".join(*args))

    @contextlib.contextmanager
    def mount_permfs(self, srcpath, mintime=1):
        with tempfile.TemporaryDirectory() as temp:
            dest = os.path.join(temp, "mount")
            os.mkdir(dest)

            log_dir = os.path.join(THIS_DIR, "logs")
            os.makedirs(log_dir, exist_ok=True)
            log = os.path.join(log_dir, "logs-%s.txt" % self.id())

            with open(log, "w") as l:
                cmd = [self.permfs, "-f", "-d", "-d", srcpath, dest]
                l.write("Command: %s\n" % cmd)
                l.flush()
                proc = subprocess.Popen(cmd, stdout=l, stderr=subprocess.STDOUT)
                try:
                    time.sleep(mintime)
                    yield dest

                finally:
                    if proc.poll() is None:
                        proc.send_signal(signal.SIGTERM)

                    proc.wait()

            self.assertEqual(proc.returncode, 7)

    def assertStat(self, path, ref_s):
        s = os.lstat(path)

        self.assertEqual(stat.filemode(s.st_mode), ref_s.mode)
        self.assertEqual(s.st_uid, ref_s.uid)
        self.assertEqual(s.st_gid, ref_s.gid)

    def assertPermFS(self, test_data, reference):
        with self.mount_permfs(os.path.join(THIS_DIR, "data", test_data)) as t:
            self.assertEqual(reference, get_stats(t))


class TestBasicPermFS(TestPermFS):
    def test_version(self):
        self.assertSubprocess([self.permfs, "--version"])

    def test_mount(self):
        with self.mount_permfs(os.path.join(THIS_DIR, "data/default")) as t:
            pass

    def test_contents(self):
        tree = os.path.join(THIS_DIR, "data/default")
        source_paths = [
            p for p in list_paths(tree) if os.path.basename(p) != "perm.yml"
        ]

        with self.mount_permfs(tree) as t:
            mount_paths = list_paths(t)

        self.assertEqual(source_paths, mount_paths)

    def test_default_perms(self):
        TEST_FILES = {
            ".": FileStat(0, 0, "drwxr-xr-x"),
            "./.hidden.txt": FileStat(0, 0, "-rw-r--r--"),
            "./file.txt": FileStat(0, 0, "-rw-r--r--"),
            "./subdir": FileStat(0, 0, "drwxr-xr-x"),
            "./subdir/file.txt": FileStat(0, 0, "-rw-r--r--"),
        }

        self.assertPermFS("default", TEST_FILES)

    def test_root_perms(self):
        TEST_FILES = {
            ".": FileStat(10, 10, "drwxrwxrwx"),
            "./file.txt": FileStat(10, 10, "-rwxrwxr-x"),
            "./subdir": FileStat(10, 10, "drwxrwxrwx"),
            "./subdir/file.txt": FileStat(10, 10, "-rwxrwxr-x"),
        }

        self.assertPermFS("root", TEST_FILES)

    def test_subdir_override_perms(self):
        TEST_FILES = {
            ".": FileStat(0, 0, "drwxr-xr-x"),
            "./file.txt": FileStat(0, 0, "-rw-r--r--"),
            "./subdir": FileStat(10, 10, "drwxrwxr-x"),
            "./subdir/file.txt": FileStat(10, 10, "-rwxrw-r-x"),
            "./subdir2": FileStat(20, 20, "drwxr-xrwx"),
            # File gets uid and gid from parent directory, but mode from root
            "./subdir2/file.txt": FileStat(20, 20, "-rwxrwxrw-"),
        }

        self.assertPermFS("subdir_override", TEST_FILES)

    def test_globs(self):
        TEST_FILES = {
            ".": FileStat(0, 0, "drwxr-xr-x"),
            "./file.txt": FileStat(0, 0, "-rw-r--r--"),
            "./file.bin": FileStat(10, 10, "-rw-rw-r--"),
            "./subdir": FileStat(0, 0, "drwxr-xr-x"),
            "./subdir/file.txt": FileStat(0, 0, "-rw-r--r--"),
            "./subdir/file.bin": FileStat(0, 0, "-rw-r--r--"),
            "./subdir2": FileStat(10, 10, "drwxr-xr-x"),
            "./subdir2/file.txt": FileStat(10, 10, "-rwxr-xr-x"),
            "./subdir2/file.bin": FileStat(10, 10, "-rwxr-xr-x"),
        }

        self.assertPermFS("globs", TEST_FILES)

    def test_mode_fallthrough(self):
        TEST_FILES = {
            ".": FileStat(10, 10, "drwxrwxr-x"),
            "./file.txt": FileStat(10, 10, "-rw-rw-r--"),
            # fmode and dmode fall through
            "./both": FileStat(20, 20, "drwxrwxr-x"),
            "./both/file.txt": FileStat(20, 20, "-rw-rw-r--"),
            "./both/subdir": FileStat(20, 20, "drwxrwxr-x"),
            "./both/subdir/file.txt": FileStat(20, 20, "-rw-rw-r--"),
            # dmode falls through
            "./dmode": FileStat(20, 20, "drwxrwxr-x"),
            "./dmode/file.txt": FileStat(20, 20, "-rw-------"),
            "./dmode/subdir": FileStat(20, 20, "drwxrwxr-x"),
            "./dmode/subdir/file.txt": FileStat(20, 20, "-rw-------"),
            # fmode falls through
            "./fmode": FileStat(20, 20, "drwxrwxrwx"),
            "./fmode/file.txt": FileStat(20, 20, "-rw-rw-r--"),
            "./fmode/subdir": FileStat(20, 20, "drwxrwxrwx"),
            "./fmode/subdir/file.txt": FileStat(20, 20, "-rw-rw-r--"),
        }

        self.assertPermFS("mode_fallthrough", TEST_FILES)

    def test_owner_fallthrough(self):
        TEST_FILES = {
            ".": FileStat(10, 10, "drwxr-xr-x"),
            "./file.txt": FileStat(10, 10, "-rw-r--r--"),
            # uid and gid fallthrough
            "./both": FileStat(10, 10, "drwxrwxr-x"),
            "./both/file.txt": FileStat(10, 10, "-rw-rw-r--"),
            "./both/subdir": FileStat(10, 10, "drwxrwxr-x"),
            "./both/subdir/file.txt": FileStat(10, 10, "-rw-rw-r--"),
            # uid fallthrough
            "./uid": FileStat(10, 20, "drwxrwxr-x"),
            "./uid/file.txt": FileStat(10, 20, "-rw-rw-r--"),
            "./uid/subdir": FileStat(10, 20, "drwxrwxr-x"),
            "./uid/subdir/file.txt": FileStat(10, 20, "-rw-rw-r--"),
            # gid fallthrough
            "./gid": FileStat(20, 10, "drwxrwxr-x"),
            "./gid/file.txt": FileStat(20, 10, "-rw-rw-r--"),
            "./gid/subdir": FileStat(20, 10, "drwxrwxr-x"),
            "./gid/subdir/file.txt": FileStat(20, 10, "-rw-rw-r--"),
        }

        self.assertPermFS("owner_fallthrough", TEST_FILES)
