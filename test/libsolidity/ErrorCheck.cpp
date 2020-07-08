// SPDX-License-Identifier: GPL-3.0
/** @file ErrorCheck.cpp
 * @author Yoichi Hirai <i@yoichihirai.com>
 * @date 2016
 */

#include <test/libsolidity/ErrorCheck.h>
#include <libsolutil/Exceptions.h>

#include <string>
#include <set>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{
std::string errorMessage(Error const& _e)
{
	return _e.comment() ? *_e.comment() : "NONE";
}
}

bool solidity::frontend::test::searchErrorMessage(Error const& _err, std::string const& _substr)
{
	if (string const* errorMessage = _err.comment())
	{
		if (errorMessage->find(_substr) == std::string::npos)
		{
			cout << "Expected message \"" << _substr << "\" but found \"" << *errorMessage << "\".\n";
			return false;
		}
		return true;
	}
	else
		cout << "Expected error message but found none." << endl;
	return _substr.empty();
}

string solidity::frontend::test::searchErrors(ErrorList const& _errors, vector<pair<Error::Type, string>> const& _expectations)
{
	auto expectations = _expectations;
	for (auto const& error: _errors)
	{
		string msg = errorMessage(*error);
		bool found = false;
		for (auto it = expectations.begin(); it != expectations.end(); ++it)
			if (msg.find(it->second) != string::npos && error->type() == it->first)
			{
				found = true;
				expectations.erase(it);
				break;
			}
		if (!found)
			return "Unexpected error: " + error->typeName() + ": " + msg;
	}
	if (!expectations.empty())
	{
		string msg = "Expected error(s) not present:\n";
		for (auto const& expectation: expectations)
			msg += expectation.second + "\n";
		return msg;
	}

	return "";
}
