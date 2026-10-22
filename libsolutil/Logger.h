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

/**
 * Runtime switchable, hierarchical logging for the compiler.
 * Loggers are named hierarchically with '.' separators, e.g. "yul.ssa.stacklayout", and are obtained lazily from LoggerRegistry.
 * Every logger starts at Level::off unless there is a parent logger of different severity.
 * Use DEFINE_LOGGER once and the solTrace/solDebug/solWarn macros at the call sites.
 */
#pragma once

#include <fmt/format.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace solidity::log
{

/// Severity levels ordered from highest to lowest verbosity.
enum class Level: std::uint8_t
{
	trace = 0,
	debug = 1,
	warn = 2,
	off = 3
};

std::string_view levelToString(Level _level) noexcept;

std::optional<Level> levelFromString(std::string_view _name);

struct LoggerHandle;

class Logger
{
public:
	bool shouldLog(Level _requested) const noexcept { return *m_level <= _requested; }

	template<typename... Args>
	void trace(fmt::format_string<Args...> _format, Args&&... _args) const
	{
		emit(Level::trace, _format, std::forward<Args>(_args)...);
	}

	template<typename... Args>
	void debug(fmt::format_string<Args...> _format, Args&&... _args) const
	{
		emit(Level::debug, _format, std::forward<Args>(_args)...);
	}

	template<typename... Args>
	void warn(fmt::format_string<Args...> _format, Args&&... _args) const
	{
		emit(Level::warn, _format, std::forward<Args>(_args)...);
	}

private:
	friend struct LoggerHandle;

	Logger() noexcept = default;
	Logger(Level const* _level, std::string_view _category) noexcept: m_level(_level), m_category(_category) {}

	template<typename... Args>
	void emit(Level _level, fmt::format_string<Args...> _format, Args&&... _args) const
	{
		emitFormatted(_level, fmt::format(_format, std::forward<Args>(_args)...));
	}

	void emitFormatted(Level _level, std::string _message) const;

	/// Points to the effective level stored in the LoggerHandle cached by the registry
	Level const* m_level = nullptr;
	std::string_view m_category;
};

 /// Process-wide singleton Registry of loggers
class LoggerRegistry
{
public:
	static LoggerRegistry& singleton();

	/// Returns the logger if it exists already.
	/// Otherwise, creates it before returning.
	Logger const& get(std::string_view _category);

	/// Sets the level for all logs with _prefix.
	void setLevel(std::string_view _prefix, Level _level);

	void setOutput(std::ostream& _stream);

	void reset();

private:
	LoggerRegistry();
	~LoggerRegistry();

	/// Returns the level set for the category.
	/// If there is no effective level set specifically for the category, searches for the closest
	/// ancestor in the hierarchy which has a level set.
	/// Returns Level::off in case nothing is found.
	Level computeEffectiveLevel(std::string_view _category) const;

	/// Writes a line prefixed with "[<level> <category>]"
	void write(Level _level, std::string_view _category, std::string const& _message);

	friend class Logger;

	std::map<std::string, Level> m_effectiveLevels;
	std::unordered_map<std::string, std::unique_ptr<LoggerHandle>> m_loggers;
	std::ostream* m_stream;
};

}

/// Defines a file logger handle bound once to dottedCategory.
/// Subsequent uses are a single reference load.
#define DEFINE_LOGGER(variableName, dottedCategory) \
	static ::solidity::log::Logger const& variableName = \
		::solidity::log::LoggerRegistry::singleton().get(dottedCategory)

#define solLog(logger, lvl, ...) \
	do { if ((logger).shouldLog(::solidity::log::Level::lvl)) [[unlikely]] \
			(logger).lvl(__VA_ARGS__); \
	} while (0)

#define solTrace(logger, ...) solLog((logger), trace, __VA_ARGS__)
#define solDebug(logger, ...) solLog((logger), debug, __VA_ARGS__)
#define solWarn(logger,  ...) solLog((logger), warn,  __VA_ARGS__)
