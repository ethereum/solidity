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
/** @file CommonIO.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "CommonIO.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#endif
#include <boost/filesystem.hpp>
#include "Exceptions.h"
using namespace std;
using namespace dev;

string dev::memDump(bytes const& _bytes, unsigned _width, bool _html)
{
	stringstream ret;
	if (_html)
		ret << "<pre style=\"font-family: Monospace,Lucida Console,Courier,Courier New,sans-serif; font-size: small\">";
	for (unsigned i = 0; i < _bytes.size(); i += _width)
	{
		ret << hex << setw(4) << setfill('0') << i << " ";
		for (unsigned j = i; j < i + _width; ++j)
			if (j < _bytes.size())
				if (_bytes[j] >= 32 && _bytes[j] < 127)
					if ((char)_bytes[j] == '<' && _html)
						ret << "&lt;";
					else if ((char)_bytes[j] == '&' && _html)
						ret << "&amp;";
					else
						ret << (char)_bytes[j];
				else
					ret << '?';
			else
				ret << ' ';
		ret << " ";
		for (unsigned j = i; j < i + _width && j < _bytes.size(); ++j)
			ret << setfill('0') << setw(2) << hex << (unsigned)_bytes[j] << " ";
		ret << "\n";
	}
	if (_html)
		ret << "</pre>";
	return ret.str();
}

template <typename _T>
inline _T contentsGeneric(std::string const& _file)
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

bytes dev::contents(string const& _file)
{
	return contentsGeneric<bytes>(_file);
}

bytesSec dev::contentsSec(string const& _file)
{
	bytes b = contentsGeneric<bytes>(_file);
	bytesSec ret(b);
	bytesRef(&b).cleanse();
	return ret;
}

string dev::contentsString(string const& _file)
{
	return contentsGeneric<string>(_file);
}

void dev::writeFile(std::string const& _file, bytesConstRef _data, bool _writeDeleteRename)
{
	namespace fs = boost::filesystem;
	if (_writeDeleteRename)
	{
		fs::path tempPath = fs::unique_path(_file + "-%%%%%%");
		writeFile(tempPath.string(), _data, false);
		// will delete _file if it exists
		fs::rename(tempPath, _file);
	}
	else
	{
		// create directory if not existent
		fs::path p(_file);
		if (!fs::exists(p.parent_path()))
		{
			fs::create_directories(p.parent_path());
			DEV_IGNORE_EXCEPTIONS(fs::permissions(p.parent_path(), fs::owner_all));
		}

		ofstream s(_file, ios::trunc | ios::binary);
		s.write(reinterpret_cast<char const*>(_data.data()), _data.size());
		if (!s)
			BOOST_THROW_EXCEPTION(FileError() << errinfo_comment("Could not write to file: " + _file));
		DEV_IGNORE_EXCEPTIONS(fs::permissions(_file, fs::owner_read|fs::owner_write));
	}
}

std::string dev::getPassword(std::string const& _prompt)
{
#if defined(_WIN32)
	cout << _prompt << flush;
	// Get current Console input flags
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	if ((hStdin = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("GetStdHandle"));
	if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("GetConsoleMode"));
	// Set console flags to no echo
	if (!SetConsoleMode(hStdin, fdwSaveOldMode & (~ENABLE_ECHO_INPUT)))
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("SetConsoleMode"));
	// Read the string
	std::string ret;
	std::getline(cin, ret);
	// Restore old input mode
	if (!SetConsoleMode(hStdin, fdwSaveOldMode))
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("SetConsoleMode"));
	return ret;
#else
	struct termios oflags;
	struct termios nflags;
	char password[256];

	// disable echo in the terminal
	tcgetattr(fileno(stdin), &oflags);
	nflags = oflags;
	nflags.c_lflag &= ~ECHO;
	nflags.c_lflag |= ECHONL;

	if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0)
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("tcsetattr"));

	printf("%s", _prompt.c_str());
	if (!fgets(password, sizeof(password), stdin))
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("fgets"));
	password[strlen(password) - 1] = 0;

	// restore terminal
	if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
		BOOST_THROW_EXCEPTION(ExternalFunctionFailure("tcsetattr"));


	return password;
#endif
}
