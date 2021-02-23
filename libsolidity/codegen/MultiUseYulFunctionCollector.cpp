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


using namespace std;
using namespace solidity;
using namespace solidity::frontend;

string MultiUseYulFunctionCollector::requestedFunctions()
{
	string result;
	for (auto const& [name, code]: m_requestedFunctions)
	{
		solAssert(code != "<<STUB<<", "");
		// std::map guarantees ascending order when iterating through its keys.
		result += code;
	}
	m_requestedFunctions.clear();
	return result;
}

string MultiUseYulFunctionCollector::createFunction(string const& _name, function<string ()> const& _creator)
{
	if (!m_requestedFunctions.count(_name))
	{
		m_requestedFunctions[_name] = "<<STUB<<";
		string fun = _creator();
		solAssert(!fun.empty(), "");
		solAssert(fun.find("function " + _name + "(") != string::npos, "Function not properly named.");
		m_requestedFunctions[_name] = std::move(fun);
	}
	return _name;
}
