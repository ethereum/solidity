@ECHO OFF

REM ---------------------------------------------------------------------------
REM Batch file for implementing release flow for solidity for Windows.
REM
REM The documentation for solidity is hosted at:
REM
REM     https://solidity.readthedocs.org
REM
REM ---------------------------------------------------------------------------
REM SPDX-License-Identifier: GPL-3.0
REM ---------------------------------------------------------------------------

set CONFIGURATION=%1
set VERSION=%2

set "DLLS=MSVC_DLLS_NOT_FOUND"
FOR /d %%d IN ("C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Redist\MSVC\*"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\*") DO set "DLLS=%%d\x86\Microsoft.VC141.CRT\msvc*.dll"

7z a solidity-windows.zip ^
    .\build\solc\%CONFIGURATION%\solc.exe .\build\test\%CONFIGURATION%\soltest.exe ^
    "%DLLS%"
