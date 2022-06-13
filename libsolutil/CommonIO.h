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
/** @file CommonIO.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * File & stream I/O routines.
 */

#pragma once

#include <boost/filesystem.hpp>

#include <libsolutil/Common.h>
#include <iostream>
#include <sstream>
#include <string>

namespace solidity
{

inline std::ostream& operator<<(std::ostream& os, bytes const& _bytes)
{
	std::ostringstream ss;
	ss << std::hex;
	std::copy(_bytes.begin(), _bytes.end(), std::ostream_iterator<int>(ss, ","));
	std::string result = ss.str();
	result.pop_back();
	os << "[" + result + "]";
	return os;
}

namespace util
{

/// Retrieves and returns the contents of the given file as a std::string.
/// If the file doesn't exist, it will throw a FileNotFound exception.
/// If the file exists but is not a regular file, it will throw NotAFile exception.
/// If the file is empty, returns an empty string.
std::string readFileAsString(boost::filesystem::path const& _file);

/// Retrieves and returns the whole content of the specified input stream (until EOF).
std::string readUntilEnd(std::istream& _stdin);

/// Tries to read exactly @a _length bytes from @a _input.
/// Returns a string containing as much data as has been read.
std::string readBytes(std::istream& _input, size_t _length);

/// Retrieves and returns a character from standard input (without waiting for EOL).
int readStandardInputChar();

/// Converts arbitrary value to string representation using std::stringstream.
template <class T>
std::string toString(T const& _t)
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
}
