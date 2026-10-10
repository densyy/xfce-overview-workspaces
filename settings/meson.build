xfce_overview_workspaces_settings_sources = [
  'general.c',
  'general.h',
  'main.c',
  'plugins.c',
  'plugins.h',
  'settings.c',
  'settings.h',
  'themes.c',
  'themes.h',
]

executable(
  'xfce-overview-workspaces-settings',
  xfce_overview_workspaces_settings_sources,
  include_directories: [
    include_directories('..'),
  ],
  c_args: [
    '-DLIBXFCE_OVERVIEW_WORKSPACES_COMPILATION=1',
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces-settings'),
  ],
  dependencies: [
    glib,
    gtk,
    libxfce4ui,
    libxfce4util,
    libxfconf_settings_dep,
    libxfce_overview_workspaces_dep,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)
