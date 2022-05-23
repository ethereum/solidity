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
	explicit FileRepository(boost::filesystem::path _basePath);

	boost::filesystem::path const& basePath() const { return m_basePath; }

	/// Translates a compiler-internal source unit name to an LSP client path.
	std::string sourceUnitNameToUri(std::string const& _sourceUnitName) const;

	/// Translates an LSP file URI into a compiler-internal source unit name.
	std::string uriToSourceUnitName(std::string const& _uri) const;

	/// @returns all sources by their compiler-internal source unit name.
	StringMap const& sourceUnits() const noexcept { return m_sourceCodes; }

	/// Changes the source identified by the LSP client path _uri to _text.
	void setSourceByUri(std::string const& _uri, std::string _text);

	void setSourceUnits(StringMap _sources);
	frontend::ReadCallback::Result readFile(std::string const& _kind, std::string const& _sourceUnitName);
	frontend::ReadCallback::Callback reader()
	{
		return [this](std::string const& _kind, std::string const& _path) { return readFile(_kind, _path); };
	}

private:
	/// Base path without URI scheme.
	boost::filesystem::path m_basePath;

	/// Additional directories used for resolving relative paths in imports.
	std::vector<boost::filesystem::path> m_includePaths;

	/// Mapping of source unit names to their URIs as understood by the client.
	StringMap m_sourceUnitNamesToUri;

	/// Mapping of source unit names to their file content.
	StringMap m_sourceCodes;
};

}
