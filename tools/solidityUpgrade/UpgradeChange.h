/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <libsolutil/AnsiColorized.h>

#include <liblangutil/SourceLocation.h>

#include <algorithm>
#include <utility>

namespace solidity::tools
{

/**
 * Models a single source code change, based on the initial source location
 * and a patch, which needs to be applied.
 * It implements the concept of level of confidence in the change and distiguishes
 * safe from unsafe changes. A "safe" change is considered to not break
 * compilation or change semantics. An "unsafe" change is considered to potentially
 * change semantics or require further manual management.
 */
class UpgradeChange
{
public:
	enum class Level
	{
		Safe,
		Unsafe
	};

	UpgradeChange(
		Level _level,
		langutil::SourceLocation _location,
		std::string _patch
	)
	:
		m_location(_location),
		m_source(_location.source->source()),
		m_patch(std::move(_patch)),
		m_level(_level) {}

	~UpgradeChange() {}

	langutil::SourceLocation const& location() { return m_location; }
	std::string source() const { return m_source; }
	std::string patch() { return m_patch; }
	Level level() const { return m_level; }

	/// Does the actual replacement of code under at current source location.
	/// The change is applied on the upgrade-specific copy of source code.
	/// The altered code is then requested by the upgrade routine later on.
	void apply();
	/// Does a pretty-print of this upgrade change. It uses a source formatter
	/// provided by the compiler in order to print affected code. Since the patch
	/// can contain a lot of code lines, it can be shortened, which is signaled
	/// by setting the flag.
	void log(bool const _shorten = true) const;
private:
	langutil::SourceLocation m_location;
	std::string m_source;
	std::string m_patch;
	Level m_level;

	/// Shortens the given source to a constant limit.
	static std::string shortenSource(std::string const& _source);
};

}
