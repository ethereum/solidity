$ErrorActionPreference = "Stop"

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

if ( -not (Test-Path "$PSScriptRoot\..\deps\boost") ) {
  New-Item -ItemType Directory -Force -Path "$PSScriptRoot\..\deps"

  Invoke-WebRequest -URI "https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-win64-x64.zip" -OutFile cmake.zip
  if ((Get-FileHash cmake.zip).Hash -ne "5f4ec834fbd9b62fbf73bc48ed459fa2ea6a86c403106c90fedc2ac76d51612d") {
    throw 'Downloaded CMake source package has wrong checksum.'
  }
  tar -xf cmake.zip
  mv cmake-3.18.2-win64-x64 "$PSScriptRoot\..\deps\cmake"

  # FIXME: The default user agent results in Artifactory treating Invoke-WebRequest as a browser
  # and serving it a page that requires JavaScript.
  Invoke-WebRequest -URI "https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.zip" -OutFile boost.zip -UserAgent ""
  if ((Get-FileHash boost.zip).Hash -ne "d2886ceff60c35fc6dc9120e8faa960c1e9535f2d7ce447469eae9836110ea77") {
    throw 'Downloaded Boost source package has wrong checksum.'
  }
  tar -xf boost.zip
  cd boost_1_77_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
}
