$ErrorActionPreference = "Stop"

cd "$PSScriptRoot\.."

if ("$Env:FORCE_RELEASE" -Or "$Env:CIRCLE_TAG") {
	New-Item prerelease.txt -type file
	Write-Host "Building release version."
}

mkdir build
cd build
$boost_dir=(Resolve-Path $PSScriptRoot\..\deps\boost\lib\cmake\Boost-*)
..\deps\cmake\bin\cmake -G "Visual Studio 16 2019" -DBoost_DIR="$boost_dir\" -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\..\upload" ..
if ( -not $? ) { throw "CMake configure failed." }
msbuild solidity.sln /p:Configuration=Release /m:5 /v:minimal
if ( -not $? ) { throw "Build failed." }
..\deps\cmake\bin\cmake --build . -j 5 --target install --config Release
if ( -not $? ) { throw "Install target failed." }
