# Building Same GNOME on Windows

This guide explains how to build Same GNOME on Windows using MSVC and vcpkg.

## Prerequisites

- Visual Studio 2019 or later (with C++ workload)
- Git for Windows
- Meson (install via `pip install meson` or from [mesonbuild.com](https://mesonbuild.com))
- Ninja (install from [ninja-build.org](https://ninja-build.org))
- vcpkg ([https://github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg))

## Step 1: Install vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

## Step 2: Build Dependencies

From the same-gnome directory root:

```powershell
vcpkg install gtk4:x64-windows cairo:x64-windows glib:x64-windows
```

## Step 3: Configure and Build

```powershell
# Set up build directory with vcpkg toolchain
meson setup builddir --backend=ninja -Dvcpkg_root=C:/path/to/vcpkg

# Build
ninja -C builddir
```

Or use the integrated vcpkg manifest mode:

```powershell
meson setup builddir --backend=ninja
ninja -C builddir
```

## Step 4: Run the Game

The executable will be at `builddir\same-gnome.exe`. You may need to copy GTK4 DLLs from vcpkg's installed directory to the build output folder, or add the vcpkg bin directory to your PATH.

## Creating a Portable ZIP Distribution

Use the provided PowerShell script to create a self-contained distribution:

```powershell
.\scripts\package-windows.ps1
```

This creates `same-gnome-windows.zip` containing the executable and all required DLLs.

## Troubleshooting

- If you get "gtk4 not found", ensure vcpkg is properly bootstrapped and dependencies are installed
- Make sure you're using the x64 Native Tools Command Prompt for VS if building with MSVC
- For development, theme files can remain in the `data/themes` directory relative to the executable