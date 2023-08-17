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
/** @file CommonIO.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libsolutil/CommonIO.h>
#include <libsolutil/Assertions.h>

#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

using namespace solidity::util;

namespace
{

template <typename T>
inline T readFile(boost::filesystem::path const& _file)
{
	assertThrow(boost::filesystem::exists(_file), FileNotFound, _file.string());

	// ifstream does not always fail when the path leads to a directory. Instead it might succeed
	// with tellg() returning a nonsensical value so that std::length_error gets raised in resize().
	assertThrow(boost::filesystem::is_regular_file(_file), NotAFile, _file.string());

	T ret;
	size_t const c_elementSize = sizeof(typename T::value_type);
	std::ifstream is(_file.string(), std::ifstream::binary);

	// Technically, this can still fail even though we checked above because FS content can change at any time.
	assertThrow(is, FileNotFound, _file.string());

	// get length of file:
	is.seekg(0, is.end);
	std::streamoff length = is.tellg();
	if (length == 0)
		return ret; // do not read empty file (MSVC does not like it)
	is.seekg(0, is.beg);

	ret.resize((static_cast<size_t>(length) + c_elementSize - 1) / c_elementSize);
	is.read(const_cast<char*>(reinterpret_cast<char const*>(ret.data())), static_cast<std::streamsize>(length));
	return ret;
}

}

std::string solidity::util::readFileAsString(boost::filesystem::path const& _file)
{
	return readFile<std::string>(_file);
}

std::string solidity::util::readUntilEnd(std::istream& _stdin)
{
	std::ostringstream ss;
	ss << _stdin.rdbuf();
	return ss.str();
}

std::string solidity::util::readBytes(std::istream& _input, size_t _length)
{
	std::string output;
	output.resize(_length);
	_input.read(output.data(), static_cast<std::streamsize>(_length));
	// If read() reads fewer bytes it sets failbit in addition to eofbit.
	if (_input.fail())
		output.resize(static_cast<size_t>(_input.gcount()));
	return output;
}

#if defined(_WIN32)
class DisableConsoleBuffering
{
public:
	DisableConsoleBuffering()
	{
		m_stdin = GetStdHandle(STD_INPUT_HANDLE);
		GetConsoleMode(m_stdin, &m_oldMode);
		SetConsoleMode(m_stdin, m_oldMode & (~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT)));
	}
	~DisableConsoleBuffering()
	{
		SetConsoleMode(m_stdin, m_oldMode);
	}
private:
	HANDLE m_stdin;
	DWORD m_oldMode;
};
#else
class DisableConsoleBuffering
{
public:
	DisableConsoleBuffering()
	{
		tcgetattr(0, &m_termios);
		m_termios.c_lflag &= ~tcflag_t(ICANON);
		m_termios.c_lflag &= ~tcflag_t(ECHO);
		m_termios.c_cc[VMIN] = 1;
		m_termios.c_cc[VTIME] = 0;
		tcsetattr(0, TCSANOW, &m_termios);
	}
	~DisableConsoleBuffering()
	{
		m_termios.c_lflag |= ICANON;
		m_termios.c_lflag |= ECHO;
		tcsetattr(0, TCSADRAIN, &m_termios);
	}
private:
	struct termios m_termios;
};
#endif

int solidity::util::readStandardInputChar()
{
	DisableConsoleBuffering disableConsoleBuffering;
	return std::cin.get();
}

std::string solidity::util::absolutePath(std::string const& _path, std::string const& _reference)
{
	boost::filesystem::path p(_path);
	// Anything that does not start with `.` is an absolute path.
	if (p.begin() == p.end() || (*p.begin() != "." && *p.begin() != ".."))
		return _path;
	boost::filesystem::path result(_reference);

	// If filename is "/", then remove_filename() throws.
	// See: https://github.com/boostorg/filesystem/issues/176
	if (result.filename() != boost::filesystem::path("/"))
		result.remove_filename();
	for (boost::filesystem::path::iterator it = p.begin(); it != p.end(); ++it)
		if (*it == "..")
			result = result.parent_path();
		else if (*it != ".")
			result /= *it;
	return result.generic_string();
}

std::string solidity::util::sanitizePath(std::string const& _path) {
	return boost::filesystem::path(_path).generic_string();
}
