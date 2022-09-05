text_engine_sources += files([
  'display.c',
  'inspector.c'
])

ui_headers = [
  'display.h',
]

gnome = import('gnome')
text_engine_sources += gnome.compile_resources('resources',
  'resources.gresource.xml',
  source_dir: '.',
  c_name: 'resources'
)

install_headers(ui_headers, subdir : header_dir / 'ui')
