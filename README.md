[![Build Status](https://travis-ci.com/garmin/fuse-permfs.svg?branch=master)](https://travis-ci.com/garmin/fuse-permfs)
# FUSE Permissions

Implements a pass through FUSE driver that reads YAML files to report different
permissions on the mounted files.

The driver looks for files named `perm.yml` which describe how the reported
permissions should be modified. For an explanation about how these files are
structured, see: [example.yml](./example.yml)

