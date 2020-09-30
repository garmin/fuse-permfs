project('permfs', 'cpp', default_options: [
  'werror=true',
  'warning_level=1',
  'buildtype=release'
])

conf_data = configuration_data()
conf_data.set_quoted('VERSION', '0.1.0')
conf_data.set('FUSE_USE_VERSION', '31')

configure_file(
  output : 'config.h',
  configuration : conf_data
)

sources = files([
  'src/dir.cpp',
  'src/file.cpp',
  'src/fs.cpp',
  'src/main.cpp',
  'src/path.cpp',
  'src/warp.cpp',
])

fuse3 = dependency('fuse3', version : '>=3.1.0')
yaml_cpp = dependency('yaml-cpp')

permfs = executable('permfs', sources, dependencies : [fuse3, yaml_cpp])

runner = find_program('test/test')
list_tests = find_program('test/list_tests.py')
r = run_command(list_tests)
foreach t: r.stdout().split()
    test(t, runner, args: ['--verbose', t], env: [
        'SEARCH_PATHS=' + permfs.full_path()
    ])
endforeach

