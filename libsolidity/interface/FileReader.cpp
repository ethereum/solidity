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
#include <libsolidity/interface/FileReader.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/Exceptions.h>

using solidity::frontend::ReadCallback;
using solidity::langutil::InternalCompilerError;
using solidity::util::errinfo_comment;
using solidity::util::readFileAsString;
using std::distance;
using std::equal;
using std::move;
using std::string;

namespace solidity::frontend
{

void FileReader::setSource(boost::filesystem::path const& _path, string _source)
{
	m_sourceCodes[_path.generic_string()] = move(_source);
}

void FileReader::setSources(StringMap _sources)
{
	m_sourceCodes = move(_sources);
}

ReadCallback::Result FileReader::readFile(string const& _kind, string const& _sourceUnitName)
{
	try
	{
		if (_kind != ReadCallback::kindString(ReadCallback::Kind::ReadFile))
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment(
				"ReadFile callback used as callback kind " +
				_kind
			));
		string strippedSourceUnitName = _sourceUnitName;
		if (strippedSourceUnitName.find("file://") == 0)
			strippedSourceUnitName.erase(0, 7);

		boost::filesystem::path canonicalPath = boost::filesystem::weakly_canonical(m_basePath / strippedSourceUnitName);
		bool isAllowed = false;
		for (boost::filesystem::path const& allowedDir: m_allowedDirectories)
		{
			// If dir is a prefix of boostPath, we are fine.
			if (
				distance(allowedDir.begin(), allowedDir.end()) <= distance(canonicalPath.begin(), canonicalPath.end()) &&
				equal(allowedDir.begin(), allowedDir.end(), canonicalPath.begin())
			)
			{
				isAllowed = true;
				break;
			}
		}
		if (!isAllowed)
			return ReadCallback::Result{false, "File outside of allowed directories."};

		if (!boost::filesystem::exists(canonicalPath))
			return ReadCallback::Result{false, "File not found."};

		if (!boost::filesystem::is_regular_file(canonicalPath))
			return ReadCallback::Result{false, "Not a valid file."};

		// NOTE: we ignore the FileNotFound exception as we manually check above
		string contents = readFileAsString(canonicalPath.string());
		m_sourceCodes[_sourceUnitName] = contents;
		return ReadCallback::Result{true, move(contents)};
	}
	catch (util::Exception const& _exception)
	{
		return ReadCallback::Result{false, "Exception in read callback: " + boost::diagnostic_information(_exception)};
	}
	catch (...)
	{
		return ReadCallback::Result{false, "Unknown exception in read callback."};
	}
}

}

