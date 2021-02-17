[![Build Status](https://github.com/garmin/fuse-permfs/workflows/build/badge.svg?branch=master&event=push)](https://github.com/garmin/fuse-permfs/actions?query=workflow%3Abuild+event%3Apush+branch%3Amaster)
# FUSE Permissions

Implements a pass through FUSE driver that reads YAML files to report different
permissions on the mounted files.

The driver looks for files named `perm.yml` which describe how the reported
permissions should be modified. For an explanation about how these files are
structured, see: [example.yml](./example.yml)

