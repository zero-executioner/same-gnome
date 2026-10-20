project('same-gnome', 'c',
  version: '1.0.0',
  default_options: ['warning_level=2', 'c_std=c11'])

gtk4_dep = dependency('gtk4', version: '>= 4.0')

src_files = files(
  'src/main.c',
  'src/game.c',
  'src/drawing.c',
)

executable('same-gnome',
  src_files,
  dependencies: [gtk4_dep],
  link_args: ['-lm'],
  install: true)

install_data('data/themes/planets.png',
  install_dir: get_option('datadir') / 'same-gnome/themes')
install_data('data/themes/stones.png',
  install_dir: get_option('datadir') / 'same-gnome/themes')
