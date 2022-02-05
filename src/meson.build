api_version = '0.1'

text_engine_sources = [
  'text-engine.c',
]

text_engine_headers = [
  'text-engine.h',
]

subdir('delta')
subdir('model')
subdir('format')
subdir('ui')

version_split = meson.project_version().split('.')
MAJOR_VERSION = version_split[0]
MINOR_VERSION = version_split[1]
MICRO_VERSION = version_split[2]

version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', MAJOR_VERSION)
version_conf.set('MINOR_VERSION', MINOR_VERSION)
version_conf.set('MICRO_VERSION', MICRO_VERSION)

configure_file(
  input: 'text-engine-version.h.in',
  output: 'text-engine-version.h',
  configuration: version_conf,
  install: true,
  install_dir: join_paths(get_option('includedir'), 'text-engine')
)

text_engine_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('json-glib-1.0'),
  dependency('libxml-2.0'),
  dependency('gtk4')
]

text_engine_lib = shared_library('text-engine-' + api_version,
  text_engine_sources,
  dependencies: text_engine_deps,
  install: true,
)

install_headers(text_engine_headers, subdir: 'text-engine')

text_engine_dep = declare_dependency(
  link_with: text_engine_lib,
  include_directories: include_directories('.'),
  dependencies: text_engine_deps,
  # sources: resources
)

pkg = import('pkgconfig')

pkg.generate(
  description: 'A shared library for ...',
    libraries: text_engine_lib,
         name: 'text-engine',
     filebase: 'text-engine-' + api_version,
      version: meson.project_version(),
      subdirs: 'text-engine',
     requires: 'glib-2.0',
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)
