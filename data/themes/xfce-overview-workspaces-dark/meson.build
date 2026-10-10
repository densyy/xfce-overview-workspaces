theme_files = [
  'animations.xml',
  'effects.xml',
  'screenshot-xfce-overview-workspaces-dark.png',
  'xfce-overview-workspaces-dark.css',
  'xfce-overview-workspaces-secondary.xml',
  'xfce-overview-workspaces.css',
  'xfce-overview-workspaces.xml',
]

themedir = get_option('prefix') / get_option('datadir') / 'themes' / 'xfce-overview-workspaces-dark' / 'xfce-overview-workspaces-1.0'

foreach theme_file : theme_files
  install_data(
    theme_file,
    install_dir: themedir,
  )
endforeach

theme_in = configure_file(
  input: 'xfce-overview-workspaces.theme.desktop.in.in',
  output: 'xfce-overview-workspaces.theme.desktop.in',
  configuration: {
    'PACKAGE_VERSION': meson.project_version(),
  },
  install: false,
)

i18n.merge_file(
  input: theme_in,
  output: 'xfce-overview-workspaces.theme',
  type: 'desktop',
  po_dir: '..' / '..' / '..' / 'po',
  install: true,
  install_dir: themedir,
)
