project(
  'jynxc', 'cpp',
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
  'src/visitor/nameresolver.cc',
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

# Parser
test_parser_exe = executable(
  'test_parser',
  'tests/test_parser.cc',
  link_with: core_lib,
  include_directories: incdir,
  dependencies: [catch2_dep],
)

test('parser', test_parser_exe, timeout: 5, suite:'parser')

# Sema
test_sema_exe = executable(
  'test_sema',
  'tests/test_sema.cc',
  link_with: core_lib,
  include_directories: incdir,
  dependencies: [catch2_dep],
)

test('sema', test_sema_exe, timeout: 5, suite:'sema')

# TypeChecker
test_typechecker_exe = executable(
  'test_typechecker',
  'tests/test_typechecker.cc',
  link_with: core_lib,
  include_directories: incdir,
  dependencies: [catch2_dep],
)

test('typechecker', test_typechecker_exe, timeout: 5, suite:'typechecker')

# Codegen
test_codegen_exe = executable(
  'test_codegen',
  'tests/test_codegen.cc',
  link_with: core_lib,
  include_directories: incdir,
  dependencies: [catch2_dep],
)

test('codegen', test_codegen_exe, timeout: 5, suite:'codegen')

# ...
