$ErrorActionPreference = "Stop"

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

  # FIXME: The default user agent results in Artifactory treating Invoke-WebRequest as a browser
  # and serving it a page that requires JavaScript.
  Invoke-WebRequest -URI "https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.zip" -OutFile boost.zip -UserAgent ""
  if ((Get-FileHash boost.zip).Hash -ne "c86bd9d9eef795b4b0d3802279419fde5221922805b073b9bd822edecb1ca28e") {
    throw 'Downloaded Boost source package has wrong checksum.'
  }
  tar -xf boost.zip
  cd boost_1_83_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
}
