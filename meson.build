project(
  'JynxCompiler', 'cpp',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'b_sanitize=address'
  ]
)

src = ['src/main.cc', 'src/lexer.cc', 'src/parser.cc', 'src/sema.cc']
incdir = include_directories('include')

core_lib = static_library(
  'jynxcore',
  ['src/lexer.cc',
  'src/parser.cc',
  'src/sema.cc',
  'src/log.cc',
  'src/token.cc',
  'src/gen.cc',
  'src/visitor/typechecker.cc',
  'src/visitor/symbolcollector.cc',
  ],
  include_directories: incdir,
)

project_target = executable(
    meson.project_name(),
    ['src/main.cc'],
    link_with: core_lib,
    include_directories : incdir,
)

# ======
# Tests
# ======
# CLI
test('no_arg', project_target, should_fail: true, suite:'cli')
test('one_arg', project_target, args : ['../examples/example.jx'], suite:'cli')

catch2_dep = dependency('catch2', fallback: ['catch2', 'catch2_with_main_dep'])

# Lexer
test_lexer_exe = executable(
  'test_lexer',
  'tests/test_lexer.cc',
  link_with: core_lib,
  include_directories: incdir,
  dependencies: [catch2_dep],
)

test('lexer', test_lexer_exe, timeout: 5, suite:'lexer')

# ...
