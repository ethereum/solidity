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
/**
 * Container of (unparsed) Yul functions identified by name which are meant to be generated
 * only once.
 */

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

string MultiUseYulFunctionCollector::requestedFunctions()
{
	string result;
	for (auto const& f: m_requestedFunctions)
		result += f.second;
	m_requestedFunctions.clear();
	return result;
}

string MultiUseYulFunctionCollector::createFunction(string const& _name, function<string ()> const& _creator)
{
	if (!m_requestedFunctions.count(_name))
	{
		string fun = _creator();
		solAssert(!fun.empty(), "");
		solAssert(fun.find("function " + _name) != string::npos, "Function not properly named.");
		m_requestedFunctions[_name] = std::move(fun);
	}
	return _name;
}
