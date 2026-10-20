# Same GNOME

A modern GTK4 port of the classic GNOME "Same" puzzle game, featuring 3D ball rotation, dynamic screen scaling, and configurable board sizes up to 256×256.

## Features

- **GTK4 Native**: Built with modern GTK4 APIs including HeaderBar menus and Cairo rendering
- **3D Ball Rotation**: Selected groups of balls rotate continuously with realistic 3D shading, moving specular highlights, and sweeping light bands
- **Dynamic Scaling**: Boards automatically scale down to fit within your screen dimensions (80% width, 60% height) when custom sizes exceed visible area
- **Configurable Sizes**: Choose from preset sizes (Small: 15×10, Medium: 30×20, Large: 45×30) or set custom dimensions up to 256×256 with 2–8 colors
- **Theme Support**: Includes planet and stone tile themes

## Building

Requires GTK4 development libraries and Meson/Ninja build system:

```bash
meson setup builddir
ninja -C builddir
```

## Running

```bash
./builddir/same-gnome
```

## Gameplay

Click on groups of 2 or more adjacent same-colored balls to remove them. The goal is to clear the entire board. Larger groups yield higher scores.

## License

This project is licensed under the GNU General Public License v2.0. See the [LICENSE](LICENSE) file for details.

The original Same game was part of gnome-games 2.22.0, Copyright © 1998–2007 Free Software Foundation, Inc.