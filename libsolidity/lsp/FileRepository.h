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
#pragma once

#include <libsolidity/interface/FileReader.h>

#include <string>
#include <map>

namespace solidity::lsp
{

class FileRepository
{
public:
	explicit FileRepository(boost::filesystem::path const& _basePath):
		m_fileReader(_basePath) {}

	boost::filesystem::path const& basePath() const { return m_fileReader.basePath(); }

	/// Translates a compiler-internal source unit name to an LSP client path.
	std::string sourceUnitNameToClientPath(std::string const& _sourceUnitName) const;
	/// Translates an LSP client path into a compiler-internal source unit name.
	std::string clientPathToSourceUnitName(std::string const& _uri) const;

	/// @returns all sources by their compiler-internal source unit name.
	std::map<std::string, std::string> const& sourceUnits() const;
	/// Changes the source identified by the LSP client path _uri to _text.
	void setSourceByClientPath(std::string const& _uri, std::string _text);

	frontend::ReadCallback::Callback reader() { return m_fileReader.reader(); }

private:
	std::map<std::string, std::string> m_sourceUnitNamesToClientPaths;
	frontend::FileReader m_fileReader;
};

}
