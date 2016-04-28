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
/** @file FileSystem.cpp
 * @authors
 *	 Eric Lombrozo <elombrozo@gmail.com>
 *	 Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "FileSystem.h"
#include "Common.h"
#include "Log.h"

#if defined(_WIN32)
#include <shlobj.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#endif
#include <boost/filesystem.hpp>
using namespace std;
using namespace dev;

// Should be written to only once during startup
static string s_ethereumDatadir;

void dev::setDataDir(string const& _dataDir)
{
	s_ethereumDatadir = _dataDir;
}

string dev::getDataDir(string _prefix)
{
	if (_prefix.empty())
		_prefix = "ethereum";
	if (_prefix == "ethereum" && !s_ethereumDatadir.empty())
		return s_ethereumDatadir;
	return getDefaultDataDir(_prefix);
}

string dev::getDefaultDataDir(string _prefix)
{
	if (_prefix.empty())
		_prefix = "ethereum";

#if defined(_WIN32)
	_prefix[0] = toupper(_prefix[0]);
	char path[1024] = "";
	if (SHGetSpecialFolderPathA(NULL, path, CSIDL_APPDATA, true))
		return (boost::filesystem::path(path) / _prefix).string();
	else
	{
	#ifndef _MSC_VER // todo?
		cwarn << "getDataDir(): SHGetSpecialFolderPathA() failed.";
	#endif
		BOOST_THROW_EXCEPTION(std::runtime_error("getDataDir() - SHGetSpecialFolderPathA() failed."));
	}
#else
	boost::filesystem::path dataDirPath;
	char const* homeDir = getenv("HOME");
	if (!homeDir || strlen(homeDir) == 0)
	{
		struct passwd* pwd = getpwuid(getuid());
		if (pwd)
			homeDir = pwd->pw_dir;
	}
	
	if (!homeDir || strlen(homeDir) == 0)
		dataDirPath = boost::filesystem::path("/");
	else
		dataDirPath = boost::filesystem::path(homeDir);
	
	return (dataDirPath / ("." + _prefix)).string();
#endif
}
