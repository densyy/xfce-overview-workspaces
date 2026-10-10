hot_corner_sources = [
  'hot-corner-settings.c',
  'hot-corner-settings.h',
  'hot-corner.c',
  'hot-corner.h',
  'plugin.c',
]

shared_module(
  'hot-corner',
  hot_corner_sources,
  name_prefix: '',
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces-plugin-hot-corner'),
    '-DPLUGIN_ID="@0@"'.format('hot-corner'),
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
