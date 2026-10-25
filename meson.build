project('same-gnome', 'c',
  version: '1.0.0',
  default_options: ['warning_level=2', 'c_std=c11'])

gnome = import('gnome')

# Detect Windows platform
is_windows = host_machine.system() == 'windows'

# Initialize Windows resources as empty list for non-Windows builds
windows_resources = []

# On Windows, use vcpkg toolchain if available
if is_windows
  # Try to find vcpkg installation
  vcpkg_root = get_option('vcpkg_root')
  if vcpkg_root != ''
    message('Using vcpkg from: ' + vcpkg_root)
  endif

  # Compile Windows resource file for version info and manifest
  winmod = import('windows')
  rc_files = files('data/same-gnome.rc')
  windows_resources = winmod.compile_resources(rc_files, name: 'same-gnome-resources')
endif

gtk4_dep = dependency('gtk4', version: '>= 4.0')

# Compile GResource bundle for theme assets
theme_resources = gnome.compile_resources('themes-resources',
  'data/themes.gresource.xml',
  c_name: 'themes',
  source_dir: 'data')

src_files = files(
  'src/main.c',
  'src/game.c',
  'src/drawing.c',
)

executable('same-gnome',
  src_files,
  theme_resources,
  windows_resources,
  dependencies: [gtk4_dep],
  link_args: ['-lm'],
  install: true)

install_data('data/themes/planets.png',
  install_dir: get_option('datadir') / 'same-gnome/themes')
install_data('data/themes/stones.png',
  install_dir: get_option('datadir') / 'same-gnome/themes')