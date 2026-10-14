$ErrorActionPreference = "Stop"

# NOTE: Changing this file invalidates the CI dependency cache on purpose, so the
# dependencies get rebuilt with the current toolchain (last: Visual Studio 2022).

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

if ( -not (Test-Path "$PSScriptRoot\..\deps\boost") ) {
  New-Item -ItemType Directory -Force -Path "$PSScriptRoot\..\deps"

  Invoke-WebRequest -URI "https://github.com/Kitware/CMake/releases/download/v3.27.4/cmake-3.27.4-windows-x86_64.zip" -OutFile cmake.zip
  if ((Get-FileHash cmake.zip).Hash -ne "e5e060756444d0b2070328a8821c1ceb62bd6d267aae61bfff06f96c7ec943a6") {
    throw 'Downloaded CMake source package has wrong checksum.'
  }
  tar -xf cmake.zip
  mv cmake-3.27.4-windows-x86_64 "$PSScriptRoot\..\deps\cmake"
  Remove-Item cmake.zip

  Invoke-WebRequest -URI "https://github.com/ccache/ccache/releases/download/v4.12.2/ccache-4.12.2-windows-x86_64.zip" -OutFile ccache.zip
  if ((Get-FileHash ccache.zip).Hash -ne "82c1b130b25cc162531dc7a062dc5ea99349cd536bc9eba8a66d976802d66516") {
    throw 'Downloaded ccache package has wrong checksum.'
  }
  tar -xf ccache.zip
  mv ccache-4.12.2-windows-x86_64 "$PSScriptRoot\..\deps\ccache"
  # ccache MSVC guide: https://github.com/ccache/ccache/wiki/MS-Visual-Studio
  # Replace ccache.exe as cl.exe so MSBuild still sees "cl.exe" while ccache wraps the real compiler.
  Copy-Item -Force "$PSScriptRoot\..\deps\ccache\ccache.exe" "$PSScriptRoot\..\deps\ccache\cl.exe"
  Remove-Item ccache.zip

  # FIXME: The default user agent results in Artifactory treating Invoke-WebRequest as a browser
  # and serving it a page that requires JavaScript.
  Invoke-WebRequest -URI "https://archives.boost.io/release/1.83.0/source/boost_1_83_0.zip" -OutFile boost.zip -UserAgent ""
  if ((Get-FileHash boost.zip).Hash -ne "c86bd9d9eef795b4b0d3802279419fde5221922805b073b9bd822edecb1ca28e") {
    throw 'Downloaded Boost source package has wrong checksum.'
  }
  tar -xf boost.zip
  Remove-Item boost.zip
  cd boost_1_83_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
  Remove-Item -LiteralPath .\boost_1_83_0 -Force -Recurse
}
