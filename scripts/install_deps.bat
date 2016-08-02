@ECHO OFF

REM ---------------------------------------------------------------------------
REM Batch file for installing pre-requisite packages for solidity on
REM Windows platforms.  That is implemented using CMake targets which
REM extract pre-built ZIPs hosted on GitHub into "deps\install_deps".
REM
REM See https://github.com/ethereum/cpp-dependencies
REM
REM The CMake files then point into that directory as an alternative
REM to the Homebrew, PPA or other global package server locations
REM available on Linux and UNIX platforms.
REM
REM The lack of a standard C++ packaging system for Windows is problematic
REM for us, and we have considered various options for improving the
REM situation, such as the following:
REM
REM     See "Windows - Add support for Chocolatey packages"
REM     https://github.com/ethereum/webthree-umbrella/issues/345
REM
REM     See "Windows - Try to use NuGet C++ packages"
REM     https://github.com/ethereum/webthree-umbrella/issues/509
REM
REM     See "CM - Can we switch to NuGet delivery for our external dependencies"
REM     https://github.com/ethereum/webthree-umbrella/issues/376
REM
REM Another possible option, which would benefit build robustness on
REM multiple platforms, not just Windows, is to add dependencies as
REM git-submodules (or downloading on demand) so that we aren'targets
REM depend on platform-specific packaging systems at all.  We have
REM already done just that for LLVM within evmjit.  The downside of
REM that approach is that those dependencies then need to be
REM built-from-source, which adds time to the build process.  It
REM gives us an unbeatable degree of control, though, because we
REM then perfectly control versioning and build flags for the binaries
REM for those packages.
REM
REM The documentation for solidity is hosted at:
REM
REM http://solidity.readthedocs.org
REM
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
REM Copyright (c) 2016 solidity contributors.
REM ---------------------------------------------------------------------------

cmake -P deps\install_deps.cmake
