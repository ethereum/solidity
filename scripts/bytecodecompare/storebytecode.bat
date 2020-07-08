@ECHO OFF

REM ---------------------------------------------------------------------------
REM SPDX-License-Identifier: GPL-3.0
REM ---------------------------------------------------------------------------

set CONFIGURATION=%1
set DIRECTORY=%2

mkdir bytecode
cd bytecode
..\scripts\isolate_tests.py ..\test\
..\scripts\bytecodecompare\prepare_report.py ..\build\solc\%CONFIGURATION%\solc.exe

REM Send to stdout instead of stderr to not confuse powershell
git clone --depth 2 git@github.com:ethereum/solidity-test-bytecode.git 2>&1
cd solidity-test-bytecode
git config user.name "travis"
git config user.email "chris@ethereum.org"
git clean -f -d -x 2>&1

if not exist %DIRECTORY% mkdir %DIRECTORY%
set REPORT=%DIRECTORY%/windows.txt
cp ../report.txt %REPORT%
git add %REPORT% 2>$1
git commit -a -m "Added report."
git pull --rebase 2>&1
git push origin 2>&1
