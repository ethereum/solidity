// SPDX-License-Identifier: GPL-3.0
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
