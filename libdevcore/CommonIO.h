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
/** @file CommonIO.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * File & stream I/O routines.
 */

#pragma once

#include <sstream>
#include <string>
#include <boost/filesystem.hpp>
#include "Common.h"

namespace dev
{

/// Retrieve and returns the contents of the given file as a std::string.
/// If the file doesn't exist or isn't readable, returns an empty container / bytes.
std::string readFileAsString(std::string const& _file);

/// Retrieve and returns the contents of standard input (until EOF).
std::string readStandardInput();

/// Retrieve and returns a character from standard input (without waiting for EOL).
int readStandardInputChar();

/// Converts arbitrary value to string representation using std::stringstream.
template <class _T>
std::string toString(_T const& _t)
{
	std::ostringstream o;
	o << _t;
	return o.str();
}

/// @returns the absolute path corresponding to @a _path relative to @a _reference.
std::string absolutePath(std::string const& _path, std::string const& _reference);

/// Helper function to return path converted strings.
std::string sanitizePath(std::string const& _path);

}
