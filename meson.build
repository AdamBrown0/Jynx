project('JynxCompiler', 'c', default_options: ['c_std=c11'])

src = ['src/main.c', 'src/lexer.c', 'src/token.c']
incdir = include_directories('include')

project_target = executable(
    meson.project_name(),
    src,
    include_directories : incdir
)

# ======
# Tests
# ======
test('no_arg', project_target, suite:'cli')
test('one_arg', project_target, args : ['../examples/example.jx'], suite:'cli')