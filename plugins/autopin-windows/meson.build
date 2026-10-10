autopin_windows_sources = [
  'autopin-windows.c',
  'autopin-windows.h',
  'plugin.c',
]

shared_module(
  'autopin-windows',
  autopin_windows_sources,
  name_prefix: '',
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces-plugin-autopin-windows'),
    '-DPLUGIN_ID="@0@"'.format('autopin-windows'),
  ],
  include_directories: [
    include_directories('..' / '..'),
  ],
  dependencies: [
    glib,
    gtk,
    libxfce4util,
    libxfce_overview_workspaces_dep,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / 'xfce-overview-workspaces' / 'plugins',
)
