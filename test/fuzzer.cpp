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
 * Executable for use with AFL <http://lcamtuf.coredump.cx/afl>.
 * Reads a single source from stdin and signals a failure for internal errors.
 */

#include <json/json.h>

#include <string>
#include <iostream>

using namespace std;

extern "C"
{
extern char const* compileJSON(char const* _input, bool _optimize);
}

string contains(string const& _haystack, vector<string> const& _needles)
{
	for (string const& needle: _needles)
		if (_haystack.find(needle) != string::npos)
			return needle;
	return "";
}

int main()
{
	string input;
	while (!cin.eof())
	{
		string s;
		getline(cin, s);
		input += s + '\n';
	}

	bool optimize = true;
	string outputString(compileJSON(input.c_str(), optimize));
	Json::Value outputJson;
	if (!Json::Reader().parse(outputString, outputJson))
	{
		cout << "Compiler produced invalid JSON output." << endl;
		abort();
	}
	if (outputJson.isMember("errors"))
	{
		if (!outputJson["errors"].isArray())
		{
			cout << "Output JSON has \"errors\" but it is not an array." << endl;
			abort();
		}
		for (Json::Value const& error: outputJson["errors"])
		{
			string invalid = contains(error.asString(), vector<string>{
				"Internal compiler error",
				"Exception during compilation",
				"Unknown exception during compilation",
				"Unknown exception while generating contract data output",
				"Unknown exception while generating formal method output",
				"Unknown exception while generating source name output",
				"Unknown error while generating JSON"
			});
			if (!invalid.empty())
			{
				cout << "Invalid error: \"" << error.asString() << "\"" << endl;
				abort();
			}
		}
	}
	else if (!outputJson.isMember("contracts"))
	{
		cout << "Output JSON has neither \"errors\" nor \"contracts\"." << endl;
		abort();
	}
	return 0;
}
