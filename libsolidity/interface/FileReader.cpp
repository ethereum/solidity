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
using solidity::util::contains;
using solidity::util::errinfo_comment;
using solidity::util::readFileAsString;
using std::distance;
using std::equal;
using std::move;
using std::string;

namespace solidity::frontend
{

FileReader::FileReader(
	boost::filesystem::path _basePath,
	FileSystemPathSet _allowedDirectories
):
	m_basePath(move(_basePath)),
	m_allowedDirectories(move(_allowedDirectories)),
	m_sourceCodes()
{
	solAssert(m_basePath.empty() || boost::filesystem::is_directory(m_basePath), "");

	// TMP: Make sure this does not add "" to allowed directories with default base path
	if (!contains(m_allowedDirectories, m_basePath))
		// TMP: This might change the case with trailing /, see comment in allowDirectory()
		allowDirectory(m_basePath);

	for (boost::filesystem::path directory: _allowedDirectories)
		allowDirectory(move(directory));
}

void FileReader::allowDirectory(boost::filesystem::path _directory)
{
	// If the given path had a trailing slash, the Boost filesystem path will have its last
	// component set to '.'. This breaks path comparison in later parts of the code, so we need
	// to strip it.
	if (_directory.filename() == ".")
		// TMP: Make sure this modifies the path in place
		_directory.remove_filename();

	m_allowedDirectories.insert(move(_directory));
}

void FileReader::allowParentDirectory(boost::filesystem::path _file)
{
	_file.remove_filename();
	allowDirectory(_file);
}

bool FileReader::inAllowedDirectory(boost::filesystem::path const& _canonicalPath) const
{
	for (boost::filesystem::path const& allowedDir: m_allowedDirectories)
	{
		// If dir is a prefix of boostPath, we are fine.
		if (
			distance(allowedDir.begin(), allowedDir.end()) <= distance(_canonicalPath.begin(), _canonicalPath.end()) &&
			equal(allowedDir.begin(), allowedDir.end(), _canonicalPath.begin())
		)
			return false;
	}

	return true;
}

void FileReader::setSource(boost::filesystem::path const& _path, string _source)
{
	m_sourceCodes[toSourceUnitName(_path)] = move(_source);
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

		boost::filesystem::path canonicalPath = fromSourceUnitName(_sourceUnitName);

		if (!inAllowedDirectory(canonicalPath))
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

string FileReader::toSourceUnitName(boost::filesystem::path const& _fsPath) const
{
	return _fsPath.generic_string();
}

boost::filesystem::path FileReader::fromSourceUnitName(string _sourceUnitName) const
{
	if (_sourceUnitName.find("file://") == 0)
		_sourceUnitName.erase(0, 7);

	return boost::filesystem::weakly_canonical(m_basePath / _sourceUnitName);
}

}
