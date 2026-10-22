Name:           same-gnome
Version:        1.0.0
Release:        1%{?dist}
Summary:        Modern GTK4 port of the classic GNOME Same puzzle game
License:        GPL-2.0-only
URL:            https://github.com/zero-executioner/same-gnome
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  gtk4-devel
BuildRequires:  pkgconfig

%description
A modern GTK4 port of the classic GNOME "Same" puzzle game, featuring
3D ball rotation, dynamic screen scaling, and configurable board sizes
up to 256x256.

Click on groups of 2 or more adjacent same-colored balls to remove them.
The goal is to clear the entire board. Larger groups yield higher scores.

%prep
%autosetup -n %{name}-%{version}

%build
%meson
%meson_build

%install
%meson_install

%files
%license LICENSE
%doc README.md
%{_bindir}/same-gnome
%{_datadir}/same-gnome/

%changelog
* Wed Oct 22 2026 djackson <djackson@sonic.danieljackson.systems> - 1.0.0-1
- Initial package release