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

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::util;

std::string MultiUseYulFunctionCollector::requestedFunctions()
{
	std::string result = std::move(m_code);
	m_code.clear();
	m_requestedFunctions.clear();
	return result;
}

std::string MultiUseYulFunctionCollector::createFunction(std::string const& _name, std::function<std::string()> const& _creator)
{
	if (!m_requestedFunctions.count(_name))
	{
		m_requestedFunctions.insert(_name);
		std::string fun = _creator();
		solAssert(!fun.empty(), "");
		solAssert(fun.find("function " + _name + "(") != std::string::npos, "Function not properly named.");
		m_code += std::move(fun);
	}
	return _name;
}

std::string MultiUseYulFunctionCollector::createFunction(
	std::string const& _name,
	std::function<std::string(std::vector<std::string>&, std::vector<std::string>&)> const& _creator
)
{
	solAssert(!_name.empty(), "");
	if (!m_requestedFunctions.count(_name))
	{
		m_requestedFunctions.insert(_name);
		std::vector<std::string> arguments;
		std::vector<std::string> returnParameters;
		std::string body = _creator(arguments, returnParameters);
		solAssert(!body.empty(), "");

		m_code += Whiskers(R"(
			function <functionName>(<args>)<?+retParams> -> <retParams></+retParams> {
				<body>
			}
		)")
		("functionName", _name)
		("args", joinHumanReadable(arguments))
		("retParams", joinHumanReadable(returnParameters))
		("body", body)
		.render();
	}
	return _name;
}
