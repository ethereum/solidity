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
/** @file CommonIO.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libdevcore/CommonIO.h>
#include <libdevcore/Assertions.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <cstdlib>
#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

using namespace std;
using namespace dev;

namespace
{

template <typename _T>
inline _T readFile(std::string const& _file)
{
	_T ret;
	size_t const c_elementSize = sizeof(typename _T::value_type);
	std::ifstream is(_file, std::ifstream::binary);
	if (!is)
		return ret;

	// get length of file:
	is.seekg(0, is.end);
	streamoff length = is.tellg();
	if (length == 0)
		return ret; // do not read empty file (MSVC does not like it)
	is.seekg(0, is.beg);

	ret.resize((length + c_elementSize - 1) / c_elementSize);
	is.read(const_cast<char*>(reinterpret_cast<char const*>(ret.data())), length);
	return ret;
}

}

string dev::readFileAsString(string const& _file)
{
	return readFile<string>(_file);
}

string dev::readStandardInput()
{
	string ret;
	while (!cin.eof())
	{
		string tmp;
		// NOTE: this will read until EOF or NL
		getline(cin, tmp);
		ret.append(tmp);
		ret.append("\n");
	}
	return ret;
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
		m_termios.c_lflag &= ~ICANON;
		m_termios.c_lflag &= ~ECHO;
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

int dev::readStandardInputChar()
{
	DisableConsoleBuffering disableConsoleBuffering;
	return cin.get();
}

string dev::absolutePath(string const& _path, string const& _reference)
{
	boost::filesystem::path p(_path);
	// Anything that does not start with `.` is an absolute path.
	if (p.begin() == p.end() || (*p.begin() != "." && *p.begin() != ".."))
		return _path;
	boost::filesystem::path result(_reference);
	result.remove_filename();
	for (boost::filesystem::path::iterator it = p.begin(); it != p.end(); ++it)
		if (*it == "..")
			result = result.parent_path();
		else if (*it != ".")
			result /= *it;
	return result.generic_string();
}

string dev::sanitizePath(string const& _path) {
	return boost::filesystem::path(_path).generic_string();
}
