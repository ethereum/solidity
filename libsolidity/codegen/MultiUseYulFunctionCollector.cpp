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
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <liblangutil/Exceptions.h>
#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>

#include <boost/core/demangle.hpp>

#include <fmt/format.h>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::util;

void MultiUseYulFunctionCollector::assertSameCreationSite(
	std::string const& _name,
	CreationSite const& _generatedAt,
	CreationSite const& _requestedAt
)
{
	static auto constexpr siteDescription = [](CreationSite const& _site) {
		return fmt::format(
			"{}:{} ({})",
			_site.file,
			_site.line,
			boost::core::demangle(_site.creator.name())
		);
	};
	solAssert(
		_generatedAt == _requestedAt,
		fmt::format(
			"Helper function {} requested from {} but generated from {}.",
			_name,
			siteDescription(_requestedAt),
			siteDescription(_generatedAt)
		)
	);
}

std::string MultiUseYulFunctionCollector::requestedFunctions()
{
	std::string result = std::move(m_code);
	m_code.clear();
	m_requestedFunctions.clear();
	return result;
}

std::string MultiUseYulFunctionCollector::createFunction(
	std::string const& _name,
	std::function<std::string()> const& _creator,
	std::source_location const _site
)
{
	return createFunctionImpl(_name, _site, _creator.target_type(), _creator);
}

std::string MultiUseYulFunctionCollector::createFunction(
	std::string const& _name,
	std::function<std::string(std::vector<std::string>&, std::vector<std::string>&)> const& _creator,
	std::source_location const _site
)
{
	solAssert(!_name.empty(), "");
	// The site is taken from the original creator, not from the adapter below.
	return createFunctionImpl(_name, _site, _creator.target_type(), [&] {
		std::vector<std::string> arguments;
		std::vector<std::string> returnParameters;
		std::string body = _creator(arguments, returnParameters);
		solAssert(!body.empty(), "");

		return Whiskers(R"(
			function <functionName>(<args>)<?+retParams> -> <retParams></+retParams> {
				<body>
			}
		)")
		("functionName", _name)
		("args", joinHumanReadable(arguments))
		("retParams", joinHumanReadable(returnParameters))
		("body", body)
		.render();
	});
}

std::string MultiUseYulFunctionCollector::createFunctionImpl(
	std::string const& _name,
	std::source_location const& _location,
	std::type_index const _creatorType,
	std::function<std::string()> const& _creator
)
{
	CreationSite const site{
		.file = _location.file_name(),
		.line = _location.line(),
		.column = _location.column(),
		.creator = _creatorType,
	};
	auto const [it, inserted] = m_requestedFunctions.try_emplace(_name, site);
	if (inserted)
	{
		std::string fun = _creator();
		solAssert(!fun.empty(), "");
		solAssert(fun.find("function " + _name + "(") != std::string::npos, "Function not properly named.");
		m_code += std::move(fun);
	}
	else
		assertSameCreationSite(_name, it->second, site);
	return _name;
}
