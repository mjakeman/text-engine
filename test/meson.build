deps = [
  dependency('glib-2.0'),
  text_engine_dep
]

tests = [
  ['move', ['move.c']],
  ['insert', ['insert.c']],
  ['delete', ['delete.c']],
  ['replace', ['replace.c']],
  ['split', ['split.c']],
  ['mark', ['mark.c']],
]

foreach t: tests
  test(
    t[0],
    executable(t[0], t[1], dependencies: deps),
    env: [
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir())
    ],
    protocol: 'tap',
  )
endforeach

