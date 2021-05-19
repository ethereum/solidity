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

#include <test/TemporaryDirectory.h>

#include <boost/filesystem.hpp>

#include <cassert>
#include <regex>
#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::test;

namespace fs = boost::filesystem;

TemporaryDirectory::TemporaryDirectory(std::string const& _prefix):
	m_path(fs::temp_directory_path() / fs::unique_path(_prefix + "%%%%-%%%%-%%%%-%%%%"))
{
	// Prefix should just be a file name and not contain anything that would make us step out of /tmp.
	assert(fs::path(_prefix) == fs::path(_prefix).stem());

	fs::create_directory(m_path);
}

TemporaryDirectory::~TemporaryDirectory()
{
	// A few paranoid sanity checks just to be extra sure we're not deleting someone's homework.
	assert(m_path.string().find(fs::temp_directory_path().string()) == 0);
	assert(!fs::equivalent(m_path, fs::temp_directory_path()));
	assert(!fs::equivalent(m_path, m_path.root_path()));
	assert(!m_path.empty());

	boost::system::error_code errorCode;
	uintmax_t numRemoved = fs::remove_all(m_path, errorCode);
	if (errorCode.value() != boost::system::errc::success)
	{
		cerr << "Failed to completely remove temporary directory '" << m_path << "'. ";
		cerr << "Only " << numRemoved << " files were actually removed." << endl;
		cerr << "Reason: " << errorCode.message() << endl;
	}
}

TemporaryWorkingDirectory::TemporaryWorkingDirectory(fs::path const& _newDirectory):
	m_originalWorkingDirectory(fs::current_path())
{
	fs::current_path(_newDirectory);
}

TemporaryWorkingDirectory::~TemporaryWorkingDirectory()
{
	fs::current_path(m_originalWorkingDirectory);
}
