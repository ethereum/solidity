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
