project(
  'wl-cairo',
  'c',
  version: '0.0.1',
  default_options: [
  	'werror=true',
  	'c_std=c11'
  ]
)

# this is for the shared memory
add_global_link_arguments('-lrt', language: 'c')

cc = meson.get_compiler('c')

#
# Dependecies
#
cairo = dependency('cairo')
realtime = cc.find_library('rt')
wayland_client = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')

deps = [
  cairo,
  realtime,
  wayland_client,
  wayland_protocols
]

# Cairo
deps += dependency('cairo')

#
# Wayland protocols
#
wl_protocols_dir = wayland_protocols.get_pkgconfig_variable('pkgdatadir')
wayland_scanner = find_program('wayland-scanner')

wl_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@-protocol.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@']
)

wl_scanner_header = generator(
  wayland_scanner,
  output: '@BASENAME@-client-protocol.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@']
)

client_protos_src = []
client_protos_headers = []

protocols_xml = [
  [wl_protocols_dir, 'stable/xdg-shell/xdg-shell.xml'],
]

foreach p : protocols_xml
  xml = join_paths(p)
  client_protos_src += wl_scanner_code.process(xml)
  client_protos_headers += wl_scanner_header.process(xml)
endforeach

lib_client_protos = static_library(
  'client_protos',
  client_protos_src + client_protos_headers
)

client_protos = declare_dependency(
  link_with: lib_client_protos,
  sources: client_protos_headers,
)

deps += client_protos

#
# Sources
#
sources = [
  'src/main.c',
  'src/shm.c',
  'src/wayland.c',
  'src/renderer.c'
]

#
# Executable
#
executable(
  'wl-cairo',
  sources,
  dependencies: deps,
)
