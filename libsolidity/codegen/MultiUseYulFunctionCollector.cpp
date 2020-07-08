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
	for (auto const& f: m_requestedFunctions)
		// std::map guarantees ascending order when iterating through its keys.
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
