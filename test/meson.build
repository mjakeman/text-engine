deps = [
  dependency('glib-2.0'),
  text_engine_dep
]

test(
  'delta',
  executable('delta', 'delta.c', dependencies: deps),
  env: [
    'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
    'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  ],
  protocol: 'tap',
)
