xfce_overview_workspaces_sources = [
  'application.c',
  'application.h',
  'main.c',
  'restart.c',
  'restart.h',
  'toolkit-init.c',
  'toolkit-init.h',
  xfce_revision_h,
]

executable(
  'xfce-overview-workspaces',
  xfce_overview_workspaces_sources,
  include_directories: [
    include_directories('..'),
  ],
  c_args: [
    '-DLIBXFCE_OVERVIEW_WORKSPACES_COMPILATION=1',
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces'),
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
