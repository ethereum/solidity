$ErrorActionPreference = "Stop"

cd "$PSScriptRoot\.."

if ("$Env:FORCE_RELEASE" -Or "$Env:CIRCLE_TAG") {
	New-Item prerelease.txt -type file
	Write-Host "Building release version."
}
else {
	# Use last commit date rather than build date to avoid ending up with builds for
	# different platforms having different version strings (and therefore producing different bytecode)
	# if the CI is triggered just before midnight.
	$last_commit_timestamp = git log -1 --date=unix --format=%cd HEAD
	$last_commit_date = (Get-Date -Date "1970-01-01 00:00:00Z").toUniversalTime().addSeconds($last_commit_timestamp).ToString("yyyy.M.d")
	-join("ci.", $last_commit_date) | out-file -encoding ascii prerelease.txt
}

mkdir build
cd build
$boost_dir=(Resolve-Path $PSScriptRoot\..\deps\boost\lib\cmake\Boost-*)
..\deps\cmake\bin\cmake -G "Visual Studio 16 2019" -DBoost_DIR="$boost_dir\" -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\..\upload" -DUSE_Z3=OFF ..
if ( -not $? ) { throw "CMake configure failed." }
msbuild solidity.sln /p:Configuration=Release /m:10 /v:minimal
if ( -not $? ) { throw "Build failed." }
..\deps\cmake\bin\cmake --build . -j 10 --target install --config Release
if ( -not $? ) { throw "Install target failed." }
