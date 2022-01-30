api_version = '0.1'

document_engine_sources = [
  'document-engine.c',
]

document_engine_headers = [
  'document-engine.h',
]

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
  input: 'document-engine-version.h.in',
  output: 'document-engine-version.h',
  configuration: version_conf,
  install: true,
  install_dir: join_paths(get_option('includedir'), 'document-engine')
)

document_engine_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
]

document_engine_lib = shared_library('document-engine-' + api_version,
  document_engine_sources,
  dependencies: document_engine_deps,
  install: true,
)

install_headers(document_engine_headers, subdir: 'document-engine')

document_engine_dep = declare_dependency(
  link_with: document_engine_lib,
  # include_directories: [],
  dependencies: document_engine_deps,
  # sources: resources
)

pkg = import('pkgconfig')

pkg.generate(
  description: 'A shared library for ...',
    libraries: document_engine_lib,
         name: 'document-engine',
     filebase: 'document-engine-' + api_version,
      version: meson.project_version(),
      subdirs: 'document-engine',
     requires: 'glib-2.0',
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)
