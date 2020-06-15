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

#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/Common.h>

#include <libsolidity/formal/ModelChecker.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/JSON.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <sstream>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace boost::unit_test;

SMTCheckerJSONTest::SMTCheckerJSONTest(string const& _filename, langutil::EVMVersion _evmVersion)
: SyntaxTest(_filename, _evmVersion)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	string jsonFilename = _filename.substr(0, _filename.size() - 4) + ".json";
	if (
		!jsonParseStrict(readFileAsString(jsonFilename), m_smtResponses) ||
		!m_smtResponses.isObject()
	)
		BOOST_THROW_EXCEPTION(runtime_error("Invalid JSON file."));

	if (ModelChecker::availableSolvers().none())
		m_shouldRun = false;
}

TestCase::TestResult SMTCheckerJSONTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	StandardCompiler compiler;

	// Run the compiler and retrieve the smtlib2queries (1st run)
	string preamble = "pragma solidity >=0.0;\n// SPDX-License-Identifier: GPL-3.0\n";
	Json::Value input = buildJson(preamble);
	Json::Value result = compiler.compile(input);

	// This is the list of query hashes requested by the 1st run
	vector<string> outHashes = hashesFromJson(result, "auxiliaryInputRequested", "smtlib2queries");

	// This is the list of responses provided in the test
	string auxInput("auxiliaryInput");
	if (!m_smtResponses.isMember(auxInput))
		BOOST_THROW_EXCEPTION(runtime_error("JSON file does not contain field \"auxiliaryInput\"."));

	vector<string> inHashes = hashesFromJson(m_smtResponses, auxInput, "smtlib2responses");

	// Ensure that the provided list matches the requested one
	if (outHashes != inHashes)
		BOOST_THROW_EXCEPTION(runtime_error(
			"SMT query hashes differ: " +
			boost::algorithm::join(outHashes, ", ") +
			" x " +
			boost::algorithm::join(inHashes, ", ")
		));

	// Rerun the compiler with the provided hashed (2nd run)
	input[auxInput] = m_smtResponses[auxInput];
	Json::Value endResult = compiler.compile(input);

	if (endResult.isMember("errors") && endResult["errors"].isArray())
	{
		Json::Value const& errors = endResult["errors"];
		for (auto const& error: errors)
		{
			if (
				!error.isMember("type") ||
				!error["type"].isString()
			)
				BOOST_THROW_EXCEPTION(runtime_error("Error must have a type."));
			if (
				!error.isMember("message") ||
				!error["message"].isString()
			)
				BOOST_THROW_EXCEPTION(runtime_error("Error must have a message."));
			if (!error.isMember("sourceLocation"))
				continue;
			Json::Value const& location = error["sourceLocation"];
			if (
				!location.isMember("start") ||
				!location["start"].isInt() ||
				!location.isMember("end") ||
				!location["end"].isInt()
			)
				BOOST_THROW_EXCEPTION(runtime_error("Error must have a SourceLocation with start and end."));
			size_t start = location["start"].asUInt();
			size_t end = location["end"].asUInt();
			std::string sourceName;
			if (location.isMember("source") && location["source"].isString())
				sourceName = location["source"].asString();
			if (start >= preamble.size())
				start -= preamble.size();
			if (end >= preamble.size())
				end -= preamble.size();
			m_errorList.emplace_back(SyntaxTestError{
				error["type"].asString(),
				error["message"].asString(),
				sourceName,
				static_cast<int>(start),
				static_cast<int>(end)
			});
		}
	}

	return conclude(_stream, _linePrefix, _formatted);
}

vector<string> SMTCheckerJSONTest::hashesFromJson(Json::Value const& _jsonObj, string const& _auxInput, string const& _smtlib)
{
	vector<string> hashes;
	Json::Value const& auxInputs = _jsonObj[_auxInput];
	if (!!auxInputs)
	{
		Json::Value const& smtlib = auxInputs[_smtlib];
		if (!!smtlib)
			for (auto const& hashString: smtlib.getMemberNames())
				hashes.push_back(hashString);
	}
	return hashes;
}

Json::Value SMTCheckerJSONTest::buildJson(string const& _extra)
{
	string language = "\"language\": \"Solidity\"";
	string sources = " \"sources\": { ";
	bool first = true;
	for (auto [sourceName, sourceContent]: m_sources)
	{
		string sourceObj = "{ \"content\": \"" + _extra + sourceContent + "\"}";
		if (!first)
			sources += ", ";
		sources += "\"" + sourceName + "\": " + sourceObj;
		first = false;
	}
	sources += "}";
	string input = "{" + language + ", " + sources + "}";
	Json::Value source;
	if (!jsonParseStrict(input, source))
		BOOST_THROW_EXCEPTION(runtime_error("Could not build JSON from string: " + input));
	return source;
}
