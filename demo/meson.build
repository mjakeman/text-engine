demo_sources = [
  'demo.c'
]

demo_deps = [
  text_engine_dep,
  dependency('libadwaita-1')
]

gnome = import('gnome')
demo_sources += gnome.compile_resources('resources',
  'resources.gresource.xml',
  source_dir: '.',
  c_name: 'resources'
)

executable('text-engine-demo', demo_sources,
  dependencies: demo_deps,
  install: true,
)
