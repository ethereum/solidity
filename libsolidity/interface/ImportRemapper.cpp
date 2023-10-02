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
#include <libsolidity/interface/ImportRemapper.h>
#include <libsolutil/CommonIO.h>
#include <liblangutil/Exceptions.h>

namespace solidity::frontend
{

void ImportRemapper::setRemappings(std::vector<Remapping> _remappings)
{
	for (auto const& remapping: _remappings)
		solAssert(!remapping.prefix.empty(), "");
	m_remappings = std::move(_remappings);
}

SourceUnitName ImportRemapper::apply(ImportPath const& _path, std::string const& _context) const
{
	// Try to find the longest prefix match in all remappings that are active in the current context.
	auto isPrefixOf = [](std::string const& _a, std::string const& _b)
	{
		if (_a.length() > _b.length())
			return false;
		return equal(_a.begin(), _a.end(), _b.begin());
	};

	size_t longestPrefix = 0;
	size_t longestContext = 0;
	std::string bestMatchTarget;

	for (auto const& redir: m_remappings)
	{
		std::string context = util::sanitizePath(redir.context);
		std::string prefix = util::sanitizePath(redir.prefix);

		// Skip if current context is closer
		if (context.length() < longestContext)
			continue;
		// Skip if redir.context is not a prefix of _context
		if (!isPrefixOf(context, _context))
			continue;
		// Skip if we already have a closer prefix match.
		if (prefix.length() < longestPrefix && context.length() == longestContext)
			continue;
		// Skip if the prefix does not match.
		if (!isPrefixOf(prefix, _path))
			continue;

		longestContext = context.length();
		longestPrefix = prefix.length();
		bestMatchTarget = util::sanitizePath(redir.target);
	}
	std::string path = bestMatchTarget;
	path.append(_path.begin() + static_cast<std::string::difference_type>(longestPrefix), _path.end());
	return path;
}

bool ImportRemapper::isRemapping(std::string_view _input)
{
	return _input.find("=") != std::string::npos;
}

	std::optional<ImportRemapper::Remapping> ImportRemapper::parseRemapping(std::string_view _input)
{
	auto equals = std::find(_input.cbegin(), _input.cend(), '=');
	if (equals == _input.end())
		return std::nullopt;

	auto const colon = std::find(_input.cbegin(), equals, ':');

	Remapping remapping{
		(colon == equals ? "" : std::string(_input.cbegin(), colon)),
		(colon == equals ? std::string(_input.cbegin(), equals) : std::string(colon + 1, equals)),
		std::string(equals + 1, _input.cend()),
	};

	if (remapping.prefix.empty())
		return std::nullopt;

	return remapping;
}

}
