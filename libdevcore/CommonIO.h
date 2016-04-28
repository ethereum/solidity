/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file CommonIO.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * File & stream I/O routines.
 */

#pragma once

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <list>
#include <memory>
#include <vector>
#include <array>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include "Common.h"
#include "Base64.h"

namespace dev
{

/// Retrieve and returns the contents of the given file as a std::string.
/// If the file doesn't exist or isn't readable, returns an empty container / bytes.
std::string contentsString(std::string const& _file);

/// Write the given binary data into the given file, replacing the file if it pre-exists.
/// Throws exception on error.
/// @param _writeDeleteRename useful not to lose any data: If set, first writes to another file in
/// the same directory and then moves that file.
void writeFile(std::string const& _file, bytesConstRef _data, bool _writeDeleteRename = false);
/// Write the given binary data into the given file, replacing the file if it pre-exists.
inline void writeFile(std::string const& _file, bytes const& _data, bool _writeDeleteRename = false) { writeFile(_file, bytesConstRef(&_data), _writeDeleteRename); }
inline void writeFile(std::string const& _file, std::string const& _data, bool _writeDeleteRename = false) { writeFile(_file, bytesConstRef(_data), _writeDeleteRename); }


#if defined(_WIN32)
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string _format = "%Y-%m-%d %H:%M:%S")
#else
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string _format = "%F %T")
#endif
{
	unsigned long milliSecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(_e.time_since_epoch()).count();
	auto const durationSinceEpoch = std::chrono::milliseconds(milliSecondsSinceEpoch);
	std::chrono::time_point<std::chrono::system_clock> const tpAfterDuration(durationSinceEpoch);

	tm timeValue;
	auto time = std::chrono::system_clock::to_time_t(tpAfterDuration);
#if defined(_WIN32)
	gmtime_s(&timeValue, &time);
#else
	gmtime_r(&time, &timeValue);
#endif

	unsigned const millisRemainder = milliSecondsSinceEpoch % 1000;
	char buffer[1024];
	if (strftime(buffer, sizeof(buffer), _format.c_str(), &timeValue))
		return std::string(buffer) + "." + (millisRemainder < 1 ? "000" : millisRemainder < 10 ? "00" : millisRemainder < 100 ? "0" : "") + std::to_string(millisRemainder) + "Z";
	return std::string();
}

// Functions that use streaming stuff.

/// Converts arbitrary value to string representation using std::stringstream.
template <class _T>
std::string toString(_T const& _t)
{
	std::ostringstream o;
	o << _t;
	return o.str();
}

}
