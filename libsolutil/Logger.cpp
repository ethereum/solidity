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

#include <libsolutil/Logger.h>

#include <iostream>

using namespace solidity::log;

namespace solidity::log
{

/// Cached logger handle owned by the registry.
/// Responsible for state changes (level) of the logger and its hierarchy.
struct LoggerHandle
{
	LoggerHandle(std::string _category, Level _level):
		category(std::move(_category)),
		effectiveLevel(_level),
		logger(&effectiveLevel, category)
	{}

	std::string category;
	Level effectiveLevel;
	Logger logger;
};

}

std::string_view solidity::log::levelToString(Level _level) noexcept
{
	switch (_level)
	{
	case Level::trace: return "trace";
	case Level::debug: return "debug";
	case Level::warn: return "warn";
	case Level::off: return "off";
	}
	return "off";
}

std::optional<Level> solidity::log::levelFromString(std::string_view _name)
{
	if (_name == "trace") return Level::trace;
	if (_name == "debug") return Level::debug;
	if (_name == "warn") return Level::warn;
	if (_name == "off") return Level::off;
	return std::nullopt;
}

LoggerRegistry& LoggerRegistry::singleton()
{
	static LoggerRegistry registry;
	return registry;
}

LoggerRegistry::LoggerRegistry():
	m_stream(&std::cerr)
{
}

LoggerRegistry::~LoggerRegistry() = default;

Logger const& LoggerRegistry::get(std::string_view _category)
{
	std::string category{_category};
	if (auto const loggerHandle = m_loggers.find(category); loggerHandle != m_loggers.end())
		return loggerHandle->second->logger;

	Level const effectiveLevel = computeEffectiveLevel(category);
	auto [newLoggerHandle, _] = m_loggers.emplace(category, std::make_unique<LoggerHandle>(category, effectiveLevel));
	return newLoggerHandle->second->logger;
}

void LoggerRegistry::setLevel(std::string_view _prefix, Level _level)
{
	m_effectiveLevels[std::string(_prefix)] = _level;

	for (auto& [category, loggerHandle]: m_loggers)
		loggerHandle->effectiveLevel = computeEffectiveLevel(loggerHandle->category);
}

void LoggerRegistry::setOutput(std::ostream& _stream)
{
	m_stream = &_stream;
}

void LoggerRegistry::reset()
{
	m_effectiveLevels.clear();
	for (auto& [category, loggerHandle]: m_loggers)
		loggerHandle->effectiveLevel = Level::off;
	m_stream = &std::cerr;
}

Level LoggerRegistry::computeEffectiveLevel(std::string_view _category) const
{
	// Search for effective level, either defined for the category itself or for its closest ancestor
	std::string key(_category);
	while (true)
	{
		if (auto const it = m_effectiveLevels.find(key); it != m_effectiveLevels.end())
			return it->second;

		auto const dot = key.rfind('.');
		if (dot == std::string::npos)
			break;
		key.resize(dot);
	}

	// Global level
	if (auto const it = m_effectiveLevels.find(""); it != m_effectiveLevels.end())
		return it->second;

	return Level::off;
}

void LoggerRegistry::write(Level _level, std::string_view _category, std::string const& _message)
{
	(*m_stream) << '[' << levelToString(_level) << ' ' << _category << "] " << _message << '\n';
}

void Logger::emitFormatted(Level _level, std::string _message) const
{
	LoggerRegistry::singleton().write(_level, m_category, _message);
}
