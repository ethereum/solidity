$ErrorActionPreference = "Stop"

cd "$PSScriptRoot\.."

mkdir build
cd build
$boost_dir=(Resolve-Path $PSScriptRoot\..\deps\boost\lib\cmake\Boost-*)

if ("$Env:FORCE_RELEASE" -Or "$Env:CIRCLE_TAG") {
	Write-Host "Building release version."
	..\deps\cmake\bin\cmake --build . -j 10 --target install --config Release -DSOL_RELEASE_VERSION="On"
}
else {
	# Use last commit date rather than build date to avoid ending up with builds for
	# different platforms having different version strings (and therefore producing different bytecode)
	# if the CI is triggered just before midnight.
	$last_commit_timestamp = git log -1 --date=unix --format=%cd HEAD
	$last_commit_date = (Get-Date -Date "1970-01-01 00:00:00Z").toUniversalTime().addSeconds($last_commit_timestamp).ToString("yyyy.M.d")
	$prerelease_string = -join("ci.", $last_commit_date)
	..\deps\cmake\bin\cmake --build . -j 10 --target install --config Release -DSOL_PRERELEASE_STRING="$prerelease_string"
}

..\deps\cmake\bin\cmake -G "Visual Studio 16 2019" -DBoost_DIR="$boost_dir\" -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\..\upload" -DUSE_Z3=OFF ..
if ( -not $? ) { throw "CMake configure failed." }
msbuild solidity.sln /p:Configuration=Release /m:10 /v:minimal
if ( -not $? ) { throw "Build failed." }
..\deps\cmake\bin\cmake --build . -j 10 --target install --config Release
if ( -not $? ) { throw "Install target failed." }
