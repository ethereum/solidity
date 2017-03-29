@ECHO OFF

REM ---------------------------------------------------------------------------
REM This file is part of solidity.
REM
REM solidity is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 3 of the License, or
REM (at your option) any later version.
REM
REM solidity is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with solidity.  If not, see <http://www.gnu.org/licenses/>
REM
REM Copyright (c) 2017 solidity contributors.
REM ---------------------------------------------------------------------------

set CONFIGURATION=%1
set COMMIT=%2

mkdir bytecode
cd bytecode
..\scripts\isolate_tests.py ..\test\
..\scripts\bytecodecompare\prepare_report.py ..\build\solc\%CONFIGURATION%\solc.exe

git clone --depth 2 git@github.com:ethereum/solidity-test-bytecode.git
cd solidity-test-bytecode
git config user.name "travis"
git config user.email "chris@ethereum.org"
git clean -f -d -x

mkdir %COMMIT%
set REPORT=%COMMIT%/windows.txt
cp ../report.txt %REPORT%
git add %REPORT%
git commit -a -m "Added report."
git push origin
