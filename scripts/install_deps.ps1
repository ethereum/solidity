$ErrorActionPreference = "Stop"

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

if ( -not (Test-Path "$PSScriptRoot\..\deps\boost") ) {
  New-Item -ItemType Directory -Force -Path "$PSScriptRoot\..\deps"

  Invoke-WebRequest -URI "https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-windows-x86_64.zip" -OutFile cmake.zip
  if ((Get-FileHash cmake.zip).Hash -ne "e28178103fad901a99fb201fac04e4787d4cd4f664c5db88998c57eed68843b9") {
    throw 'Downloaded CMake source package has wrong checksum.'
  }
  tar -xf cmake.zip
  mv cmake-3.21.3-windows-x86_64 "$PSScriptRoot\..\deps\cmake"
  Remove-Item cmake.zip

  # FIXME: The default user agent results in Artifactory treating Invoke-WebRequest as a browser
  # and serving it a page that requires JavaScript.
  Invoke-WebRequest -URI "https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.zip" -OutFile boost.zip -UserAgent ""
  if ((Get-FileHash boost.zip).Hash -ne "f22143b5528e081123c3c5ed437e92f648fe69748e95fa6e2bd41484e2986cc3") {
    throw 'Downloaded Boost source package has wrong checksum.'
  }
  tar -xf boost.zip
  Remove-Item boost.zip
  cd boost_1_78_0
  .\bootstrap.bat
  .\b2 -j4 -d0 link=static runtime-link=static variant=release threading=multi address-model=64 --with-filesystem --with-system --with-program_options --with-test --prefix="$PSScriptRoot\..\deps\boost" install
  if ( -not $? ) { throw "Error building boost." }
  cd ..
  Remove-Item -LiteralPath .\boost_1_78_0 -Force -Recurse
}
