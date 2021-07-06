$ErrorActionPreference = "Stop"

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

if ( -not (Test-Path "$PSScriptRoot\..\deps\boost") ) {
  New-Item -ItemType Directory -Force -Path "$PSScriptRoot\..\deps"

  Invoke-WebRequest -URI "https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-win64-x64.zip" -OutFile cmake.zip
  tar -xf cmake.zip
  mv cmake-3.18.2-win64-x64 "$PSScriptRoot\..\deps\cmake"

  Invoke-WebRequest -URI "https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.zip" -OutFile boost.zip
  tar -xf boost.zip
  cd boost_1_74_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
}
