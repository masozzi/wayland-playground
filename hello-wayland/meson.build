project (
  'hello-wayland',
  'c',
  version: '0.0.1',
  default_options: [
  	'werror=true',
  	'c_std=c11'
  ]
)

command_h = run_command('wayland-scanner', 'client-header',
  'protocols/xdg-shell.xml', 'src/xdg-shell-client-protocol.h')
if command_h.returncode() != 0
  error('cannot create header file')
endif

command_c = run_command('wayland-scanner', 'private-code',
  'protocols/xdg-shell.xml', 'src/xdg-shell-protocol.c')
if command_c.returncode() != 0
  error('cannot create c file')
endif

sources = [
  'src/main.c',
  'src/shm.c',
  'src/xdg-shell-client-protocol.h',
  'src/xdg-shell-protocol.c'
]

deps = [
  dependency('wayland-client'),
  dependency('wayland-scanner')
]

executable (
  'hello-wayland',
  sources,
  dependencies: deps,
  link_args: '-lrt'
)
