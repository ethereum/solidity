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

#include <libsolidity/lsp/FileRepository.h>

using namespace std;
using namespace solidity;
using namespace solidity::lsp;

namespace
{

string stripFilePrefix(string const& _path)
{
	if (_path.find("file://") == 0)
		return _path.substr(7);
	else
		return _path;
}

}

string FileRepository::sourceUnitNameToClientPath(string const& _sourceUnitName) const
{
	if (m_sourceUnitNamesToClientPaths.count(_sourceUnitName))
		return m_sourceUnitNamesToClientPaths.at(_sourceUnitName);
	else if (_sourceUnitName.find("file://") == 0)
		return _sourceUnitName;
	else
		return "file://" + (m_fileReader.basePath() / _sourceUnitName).generic_string();
}

string FileRepository::clientPathToSourceUnitName(string const& _path) const
{
	return m_fileReader.cliPathToSourceUnitName(stripFilePrefix(_path));
}

map<string, string> const& FileRepository::sourceUnits() const
{
	return m_fileReader.sourceUnits();
}

void FileRepository::setSourceByClientPath(string const& _uri, string _text)
{
	// This is needed for uris outside the base path. It can lead to collisions,
	// but we need to mostly rewrite this in a future version anyway.
	m_sourceUnitNamesToClientPaths.emplace(clientPathToSourceUnitName(_uri), _uri);
	m_fileReader.addOrUpdateFile(stripFilePrefix(_uri), move(_text));
}
