/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceLocation.h>

#include <cstddef>
#include <string>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::evmasm
{

/// Reports warnings, via @a _errorReporter, if the given bytecode sizes exceed the limits.
/// @param _creationCodeSize size in bytes of the assembled creation (init) code.
/// @param _runtimeCodeSize size in bytes of the assembled runtime code.
void checkCodeSizeLimits(
	langutil::ErrorReporter& _errorReporter,
	langutil::EVMVersion _evmVersion,
	langutil::SourceLocation const& _location,
	std::string const& _codeSizeWarningSubject,
	size_t _creationCodeSize,
	size_t _runtimeCodeSize
);

}
