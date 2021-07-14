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

#include <optional>
#include <string>
#include <vector>

namespace solidity::frontend
{

// Some helper typedefs to make reading the signatures more self explaining.
using SourceUnitName = std::string;
using SourceCode = std::string;
using ImportPath = std::string;

/// The ImportRemapper is being used on imported file paths for being remapped to source unit IDs before being loaded.
class ImportRemapper
{
public:
	struct Remapping
	{
		bool operator!=(Remapping const& _other) const noexcept { return !(*this == _other); }
		bool operator==(Remapping const& _other) const noexcept
		{
			return
				context == _other.context &&
				prefix == _other.prefix &&
				target == _other.target;
		}

		std::string context;
		std::string prefix;
		std::string target;
	};

	void clear() { m_remappings.clear(); }

	void setRemappings(std::vector<Remapping> _remappings);
	std::vector<Remapping> const& remappings() const noexcept { return m_remappings; }

	SourceUnitName apply(ImportPath const& _path, std::string const& _context) const;

	// Parses a remapping of the format "context:prefix=target".
	static std::optional<Remapping> parseRemapping(std::string const& _remapping);

private:
	/// list of path prefix remappings, e.g. mylibrary: github.com/ethereum = /usr/local/ethereum
	/// "context:prefix=target"
	std::vector<Remapping> m_remappings = {};
};

}
