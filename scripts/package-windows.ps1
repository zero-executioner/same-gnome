# Same GNOME Windows Packaging Script
# Creates a portable ZIP distribution with all required DLLs

$ErrorActionPreference = "Stop"

# Configuration
$BuildDir = "builddir"
$StagingDir = "dist-staging"
$OutputZip = "same-gnome-windows.zip"
$VcpkgRoot = $env:VCPKG_ROOT
if (-not $VcpkgRoot) {
    Write-Warning "VCPKG_ROOT environment variable not set. Attempting to find vcpkg..."
    # Try common locations
    $CommonPaths = @(
        "$env:USERPROFILE\vcpkg",
        "C:\vcpkg",
        "$env:LOCALAPPDATA\vcpkg"
    )
    foreach ($path in $CommonPaths) {
        if (Test-Path "$path\installed\x64-windows\bin") {
            $VcpkgRoot = $path
            Write-Host "Found vcpkg at: $VcpkgRoot"
            break
        }
    }
    if (-not $VcpkgRoot) {
        Write-Error "Could not find vcpkg installation. Please set VCPKG_ROOT environment variable."
        exit 1
    }
}

Write-Host "=== Same GNOME Windows Packaging ==="
Write-Host "vcpkg root: $VcpkgRoot"

# Step 1: Build in Release mode
Write-Host "`n[1/4] Building in Release mode..."
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
meson setup $BuildDir --buildtype=release
ninja -C $BuildDir

if (-not (Test-Path "$BuildDir\same-gnome.exe")) {
    Write-Error "Build failed: same-gnome.exe not found in $BuildDir"
    exit 1
}

# Step 2: Create staging directory
Write-Host "`n[2/4] Creating staging directory..."
if (Test-Path $StagingDir) {
    Remove-Item -Recurse -Force $StagingDir
}
New-Item -ItemType Directory -Path $StagingDir | Out-Null

# Copy executable
Copy-Item "$BuildDir\same-gnome.exe" $StagingDir

# Copy theme data
New-Item -ItemType Directory -Path "$StagingDir\data\themes" -Force | Out-Null
Copy-Item "data\themes\*.png" "$StagingDir\data\themes\"

# Step 3: Copy required GTK4 DLLs from vcpkg
Write-Host "`n[3/4] Copying GTK4 DLLs from vcpkg..."
$DllSource = "$VcpkgRoot\installed\x64-windows\bin"
$RequiredDlls = @(
    "gtk-4.dll",
    "gdk-4.dll",
    "cairo.dll",
    "glib-2.0.dll",
    "gobject-2.0.dll",
    "gio-2.0.dll",
    "pango-1.0.dll",
    "pangocairo-1.0.dll",
    "pangoft2-1.0.dll",
    "fontconfig.dll",
    "freetype.dll",
    "harfbuzz.dll",
    "graphene-1.0.dll",
    "epoxy.dll",
    "intl.dll",
    "ffi-8.dll",
    "zlib1.dll",
    "png16.dll",
    "jpeg.dll",
    "tiff.dll",
    "webp.dll",
    "pixman-1.dll",
    "expat.dll",
    "brotlidec.dll",
    "brotlicommon.dll",
    "fribidi.dll",
    "thai.dll",
    "iconv.dll"
)

$CopiedCount = 0
foreach ($dll in $RequiredDlls) {
    $sourcePath = Join-Path $DllSource $dll
    if (Test-Path $sourcePath) {
        Copy-Item $sourcePath $StagingDir -Force
        $CopiedCount++
    } else {
        Write-Warning "DLL not found: $dll (this may be optional)"
    }
}
Write-Host "Copied $CopiedCount DLLs"

# Also copy any additional DLLs that might be needed
Get-ChildItem "$DllSource\*.dll" | Where-Object { $_.Name -notin $RequiredDlls } | ForEach-Object {
    Copy-Item $_.FullName $StagingDir -Force
}

# Step 4: Create ZIP archive
Write-Host "`n[4/4] Creating ZIP archive..."
if (Test-Path $OutputZip) {
    Remove-Item $OutputZip
}
Compress-Archive -Path "$StagingDir\*" -DestinationPath $OutputZip

Write-Host "`n=== Packaging Complete ==="
Write-Host "Output: $OutputZip"
Write-Host "Size: $((Get-Item $OutputZip).Length / 1MB) MB"
Write-Host "`nTo distribute, simply extract the ZIP and run same-gnome.exe"