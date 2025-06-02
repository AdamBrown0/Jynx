project('JynxCompiler', 'c', default_options: ['c_std=c11'])

src = ['src/main.c', 'src/lexer.c', 'src/token.c']
incdir = include_directories('include')

core_lib = static_library(
  'jynxcore',
  ['src/lexer.c', 'src/token.c'],
  include_directories: incdir,
)

project_target = executable(
    meson.project_name(),
    ['src/main.c'],
    link_with: core_lib,
    include_directories : incdir,
)

# ======
# Tests
# ======
# CLI
test('no_arg', project_target, should_fail: true, suite:'cli')
test('one_arg', project_target, args : ['../examples/example.jx'], suite:'cli')

# Lexer
test_lexer_exe = executable(
  'test_lexer',
  'tests/test_lexer.c',
  link_with: core_lib,
  include_directories: incdir,
)

test('lexer', test_lexer_exe, timeout: 5, suite:'lexer')

# ...
