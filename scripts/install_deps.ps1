$ErrorActionPreference = "Stop"

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

if ( -not (Test-Path "$PSScriptRoot\..\deps\boost") ) {
  New-Item -ItemType Directory -Force -Path "$PSScriptRoot\..\deps"

  Invoke-WebRequest -URI "https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3-windows-x86_64.zip" -OutFile cmake.zip
  if ((Get-FileHash cmake.zip).Hash -ne "0018b369f06646d4784fad131a155333c9d59a03dee6390324f81b9df72a2f00") {
    throw 'Downloaded CMake source package has wrong checksum.'
  }
  tar -xf cmake.zip
  mv cmake-3.22.3-windows-x86_64 "$PSScriptRoot\..\deps\cmake"

  # FIXME: The default user agent results in Artifactory treating Invoke-WebRequest as a browser
  # and serving it a page that requires JavaScript.
  Invoke-WebRequest -URI "https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.zip" -OutFile boost.zip -UserAgent ""
  if ((Get-FileHash boost.zip).Hash -ne "f22143b5528e081123c3c5ed437e92f648fe69748e95fa6e2bd41484e2986cc3") {
    throw 'Downloaded Boost source package has wrong checksum.'
  }
  tar -xf boost.zip
  cd boost_1_78_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
}
