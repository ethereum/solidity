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

#include <liblangutil/SourceReferenceFormatter.h>
#include <libsolidity/ast/ASTJsonExporter.h>
#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/JSON.h>

#include <test/Common.h>
#include <test/libsolidity/ASTJSONPropertyTest.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util::formatting;
using namespace solidity::util;
using namespace solidity;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

namespace
{

string const sourceDelimiter("==== Source: ");

}

void ASTJSONPropertyTest::fillSources(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string sourceName;
	string source;
	string line;
	string const delimiter("// ----");
	while (getline(file, line))
	{
		if (boost::algorithm::starts_with(line, sourceDelimiter))
		{
			if (!sourceName.empty())
				m_sources.emplace_back(sourceName, source);

			sourceName = line.substr(
				sourceDelimiter.size(),
				line.size() - " ===="s.size() - sourceDelimiter.size()
			);
			source = string();
		}
		else if (!line.empty() && !boost::algorithm::starts_with(line, delimiter))
			source += line + "\n";
	}
	m_sources.emplace_back(sourceName.empty() ? "a" : sourceName, source);
	file.close();
}

ASTJSONPropertyTest::ASTJSONPropertyTest(string const& _filename)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	fillSources(_filename);
}

TestCase::TestResult ASTJSONPropertyTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	CompilerStack c;

	StringMap sources;
	map<string, unsigned> sourceIndices;
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		sources[m_sources[i].first] = m_sources[i].second;
		sourceIndices[m_sources[i].first] = static_cast<unsigned>(i + 1);
	}

	c.reset();
	c.setSources(sources);
	c.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());

	if (!c.parseAndAnalyze())
		return TestResult::FatalError;

	return runTest(
		sourceIndices,
		c,
		_stream,
		_linePrefix,
		_formatted
	) ? TestResult::Success : TestResult::Failure;
}

bool ASTJSONPropertyTest::checkASTProperty(std::string _property, Json::Value const& _node)
{
	Json::Value properties;
	std::string error;
	if (!jsonParseStrict(_property, properties, &error))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test property: \"" + _property + "\": " + error));

	for (auto propertyName: properties.getMemberNames())
	{
		if (_node[propertyName] != properties[propertyName])
		{
			std::cout << jsonCompactPrint(_node[propertyName]) << " != " << jsonCompactPrint(properties[propertyName]) << std::endl;
			return false;
		}
	}

	return true;
}

bool ASTJSONPropertyTest::runTest(
	map<string, unsigned> const& _sourceIndices,
	CompilerStack& _compiler,
	ostream& /*_stream*/,
	string const& /*_linePrefix*/,
	bool const /*_formatted*/
)
{
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		auto jsonAST = ASTJsonExporter(_compiler.state(), _sourceIndices).toJson(_compiler.ast(m_sources[i].first));

		auto walkAST = [this](Json::Value const& _node, auto _recurse) -> bool {
			if (_node.type() == Json::ValueType::objectValue && _node.isMember("documentation"))
			{
				std::string docString = _node.get("documentation", {}).asString();
				static constexpr std::string_view testMarker = "@custom:test ";
				if (boost::starts_with(docString, testMarker))
					if (!checkASTProperty(docString.substr(testMarker.size()), _node))
						return false;
			}
			for (auto const& member: _node)
				if (!_recurse(member, _recurse))
					return false;
			return true;
		};

		if (!walkAST(jsonAST, walkAST))
			return false;
	}

	return true;
}

void ASTJSONPropertyTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	for (auto const& source: m_sources)
	{
		if (m_sources.size() > 1 || source.first != "a")
			_stream << _linePrefix << sourceDelimiter << source.first << " ====" << endl << endl;
		stringstream stream(source.second);
		string line;
		while (getline(stream, line))
			_stream << _linePrefix << line << endl;
		_stream << endl;
	}
}

void ASTJSONPropertyTest::printUpdatedExpectations(std::ostream&, std::string const&) const
{
}

void ASTJSONPropertyTest::updateExpectation(string const& _filename, string const& _expectation, string const& _variant) const
{
	ofstream file(_filename.c_str());
	if (!file) BOOST_THROW_EXCEPTION(runtime_error("Cannot write " + _variant + "AST expectation to \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string replacedResult = _expectation;

	file << replacedResult;
	file.flush();
	file.close();
}
