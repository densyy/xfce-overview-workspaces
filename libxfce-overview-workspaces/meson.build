libxfce_overview_workspaces_headers = [
  'action-button.h',
  'actor.h',
  'animation.h',
  'application-button.h',
  'application-database.h',
  'applications-menu-model.h',
  'applications-view.h',
  'application-tracker.h',
  'background.h',
  'binding.h',
  'bindings-pool.h',
  'box-layout.h',
  'button.h',
  'click-action.h',
  'collapse-box.h',
  'core.h',
  'css-selector.h',
  'debug.h',
  'desktop-app-info-action.h',
  'desktop-app-info.h',
  'drag-action.h',
  'drop-action.h',
  'dynamic-table-layout.h',
  'emblem-effect.h',
  'fill-box-layout.h',
  'focusable.h',
  'focus-manager.h',
  'gradient-color.h',
  'image-content.h',
  'label.h',
  'libxfce-overview-workspaces.h',
  'live-window.h',
  'live-window-simple.h',
  'live-window-snapshot.h',
  'live-workspace.h',
  'model.h',
  'outline-effect.h',
  'plugin.h',
  'plugin-settings.h',
  'plugins-manager.h',
  'popup-menu.h',
  'popup-menu-item-button.h',
  'popup-menu-item.h',
  'popup-menu-item-separator.h',
  'scaled-table-layout.h',
  'scrollbar.h',
  'settings.h',
  'stage.h',
  'stage-interface.h',
  'stylable.h',
  'text-box.h',
  'theme-animation.h',
  'theme-css.h',
  'theme-effects.h',
  'theme.h',
  'theme-layout.h',
  'toggle-button.h',
  'tooltip-action.h',
  'transition-group.h',
  'types.h',
  'utils.h',
  'view.h',
  'view-manager.h',
  'viewpad.h',
  'window-content.h',
  'windows-view.h',
  'window-tracker-backend.h',
  'window-tracker.h',
  'window-tracker-monitor.h',
  'window-tracker-window.h',
  'window-tracker-workspace.h',
  'workspace-selector.h',
]

libxfce_overview_workspaces_headers += configure_file(
  input: 'version.h.in',
  output: 'version.h',
  configuration: {
    'LIBXFCE_OVERVIEW_WORKSPACES_MAJOR_VERSION': version_parts[0],
    'LIBXFCE_OVERVIEW_WORKSPACES_MINOR_VERSION': version_parts[1],
    'LIBXFCE_OVERVIEW_WORKSPACES_MICRO_VERSION': version_parts[2],
  },
)

libxfce_overview_workspaces_sources = [
  'action-button.c',
  'actor.c',
  'animation.c',
  'application-button.c',
  'application-database.c',
  'applications-menu-model.c',
  'applications-view.c',
  'application-tracker.c',
  'background.c',
  'binding.c',
  'bindings-pool.c',
  'box-layout.c',
  'button.c',
  'click-action.c',
  'collapse-box.c',
  'core.c',
  'css-selector.c',
  'debug.c',
  'desktop-app-info-action.c',
  'desktop-app-info.c',
  'drag-action.c',
  'drop-action.c',
  'dynamic-table-layout.c',
  'emblem-effect.c',
  'fill-box-layout.c',
  'focusable.c',
  'focus-manager.c',
  'gradient-color.c',
  'image-content.c',
  'label.c',
  'live-window.c',
  'live-window-simple.c',
  'live-window-snapshot.c',
  'live-workspace.c',
  'model.c',
  'outline-effect.c',
  'plugin.c',
  'plugin-settings.c',
  'plugins-manager.c',
  'popup-menu.c',
  'popup-menu-item-button.c',
  'popup-menu-item.c',
  'popup-menu-item-separator.c',
  'scaled-table-layout.c',
  'scrollbar.c',
  'settings.c',
  'stage.c',
  'stage-interface.c',
  'stylable.c',
  'text-box.c',
  'theme-animation.c',
  'theme.c',
  'theme-css.c',
  'theme-effects.c',
  'theme-layout.c',
  'toggle-button.c',
  'tooltip-action.c',
  'transition-group.c',
  'utils.c',
  'view.c',
  'view-manager.c',
  'viewpad.c',
  'window-content.c',
  'windows-view.c',
  'window-tracker-backend.c',
  'window-tracker.c',
  'window-tracker-monitor.c',
  'window-tracker-window.c',
  'window-tracker-workspace.c',
  'workspace-selector.c',
]

libxfce_overview_workspaces_sources += gnome.mkenums(
  'enums',
  c_template: 'enums.c.template',
  h_template: 'enums.h.template',
  sources: libxfce_overview_workspaces_headers,
  identifier_prefix: 'XfceOverviewWorkspaces',
  symbol_prefix: 'xfce_overview_workspaces',
)

libxfce_overview_workspaces_sources += gnome.genmarshal(
  'marshal',
  sources: 'marshal.list',
  prefix: '_xfce_overview_workspaces_marshal',
  internal: true,
  install_header: false,
)

if x11.found()
  libxfce_overview_workspaces_headers += [
    'x11' / 'thumbnail-cache.h',
    'x11' / 'window-content-frozen.h',
    'x11' / 'window-content-x11.h',
    'x11' / 'window-tracker-backend-x11.h',
    'x11' / 'window-tracker-monitor-x11.h',
    'x11' / 'window-tracker-window-x11.h',
    'x11' / 'window-tracker-workspace-x11.h',
    'x11' / 'window-tracker-x11.h',
  ]

  libxfce_overview_workspaces_sources += [
    'x11' / 'thumbnail-cache.c',
    'x11' / 'window-content-frozen.c',
    'x11' / 'window-content-x11.c',
    'x11' / 'window-tracker-backend-x11.c',
    'x11' / 'window-tracker-monitor-x11.c',
    'x11' / 'window-tracker-window-x11.c',
    'x11' / 'window-tracker-workspace-x11.c',
    'x11' / 'window-tracker-x11.c',
  ]
endif

if clutter_gdk.found()
  libxfce_overview_workspaces_headers += [
    'gdk' / 'window-tracker-backend-gdk.h',
  ]

  libxfce_overview_workspaces_sources += [
    'gdk' / 'window-tracker-backend-gdk.c',
  ]
endif

install_headers(
  libxfce_overview_workspaces_headers,
  subdir: 'xfce-overview-workspaces' / 'libxfce-overview-workspaces',
)

libxfce_overview_workspaces_version = '0'
libxfce_overview_workspaces_so_version = '@0@.10100.0'.format(libxfce_overview_workspaces_version)

libxfce_overview_workspaces_deps = [
  clutter,
  clutter_cogl,
  clutter_gdk,
  cogl,
  garcon,
  gio,
  gio_unix,
  glib,
  gtk,
  libwnck,
  libxfce4ui,
  libxfce4util,
  x11,
  xcomposite,
  xdamage,
  xinerama,
]

libxfce_overview_workspaces = library(
  'xfce-overview-workspaces',
  [
    libxfce_overview_workspaces_headers,
    libxfce_overview_workspaces_sources,
    xfce_revision_h,
  ],
  version: libxfce_overview_workspaces_version,
  soversion: libxfce_overview_workspaces_so_version,
  c_args: [
    '-DLIBXFCE_OVERVIEW_WORKSPACES_COMPILATION=1',
    '-DG_LOG_DOMAIN="@0@"'.format('xfce-overview-workspaces'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: libxfce_overview_workspaces_deps,
  install: true,
)

pkgconfig.generate(
  libraries: [
    libxfce_overview_workspaces,
  ],
  version: meson.project_version(),
  name: 'xfce-overview-workspaces',
  filebase: 'libxfce-overview-workspaces',
  description: 'Library for xfce-overview-workspaces',
  subdirs: [
    'xfce-overview-workspaces',
  ],
  variables: [
    'exec_prefix=@0@'.format('${prefix}'),
  ],
  install_dir: get_option('prefix') / get_option('libdir') / 'pkgconfig',
)

libxfce_overview_workspaces_dep = declare_dependency(
  dependencies: libxfce_overview_workspaces_deps,
  link_with: libxfce_overview_workspaces,
)
