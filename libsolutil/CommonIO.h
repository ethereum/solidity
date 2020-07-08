// SPDX-License-Identifier: GPL-3.0
/** @file CommonIO.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * File & stream I/O routines.
 */

#pragma once

#include <libsolutil/Common.h>
#include <sstream>
#include <string>

namespace solidity::util
{

/// Retrieve and returns the contents of the given file as a std::string.
/// If the file doesn't exist or isn't readable, returns an empty container / bytes.
std::string readFileAsString(std::string const& _file);

/// Retrieve and returns the contents of standard input (until EOF).
std::string readStandardInput();

/// Retrieve and returns a character from standard input (without waiting for EOL).
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
