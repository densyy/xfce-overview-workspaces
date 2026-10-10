clock_view_sources = [
  'clock-view-settings.c',
  'clock-view-settings.h',
  'clock-view.c',
  'clock-view.h',
  'plugin.c',
]

shared_module(
  'clock-view',
  clock_view_sources,
  name_prefix: '',
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces-plugin-clock-view'),
    '-DPLUGIN_ID="@0@"'.format('clock-view'),
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
