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
 * Unit tests for the solidity compiler ABI JSON Interface output.
 */

#include <test/libsolidity/NatspecJSONTest.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/predicate.hpp>

#include <fmt/format.h>

#include <vector>

using namespace solidity::frontend::test;
using namespace solidity::util;
using namespace std::string_literals;

std::ostream& solidity::frontend::test::operator<<(std::ostream& _output, NatspecJSONKind _kind)
{
	switch (_kind) {
	case NatspecJSONKind::Devdoc: _output << "devdoc"; break;
	case NatspecJSONKind::Userdoc: _output << "userdoc"; break;
	}
	return _output;
}

std::unique_ptr<TestCase> NatspecJSONTest::create(Config const& _config)
{
	return std::make_unique<NatspecJSONTest>(_config.filename, _config.evmVersion);
}

void NatspecJSONTest::parseCustomExpectations(std::istream& _stream)
{
	soltestAssert(m_expectedNatspecJSON.empty());

	// We expect a series of expectations in the following format:
	//
	//     // <qualified contract name> <devdoc|userdoc>
	//     // <json>

	std::string line;
	while (getline(_stream, line))
	{
		std::string_view strippedLine = expectLinePrefix(line);
		if (strippedLine.empty())
			continue;

		auto [contractName, kind] = parseExpectationHeader(strippedLine);

		std::string rawJSON = extractExpectationJSON(_stream);
		std::string jsonErrors;
		Json::Value parsedJSON;
		bool jsonParsingSuccessful = jsonParseStrict(rawJSON, parsedJSON, &jsonErrors);
		if (!jsonParsingSuccessful)
			BOOST_THROW_EXCEPTION(std::runtime_error(fmt::format(
				"Malformed JSON in {} expectation for contract {}.\n"
				"Note that JSON expectations must be pretty-printed to be split correctly. "
				"The object is assumed to and at the first unindented closing brace.\n"
				"{}",
				toString(kind),
				contractName,
				rawJSON
			)));

		m_expectedNatspecJSON[std::string(contractName)][kind] = parsedJSON;
	}
}

bool NatspecJSONTest::expectationsMatch()
{
	// NOTE: Comparing pretty printed Json::Values to avoid using its operator==, which fails to
	// compare equal numbers as equal. For example, for 'version' field the value is sometimes int,
	// sometimes uint and they compare as different even when both are 1.
	return
		SyntaxTest::expectationsMatch() &&
		prettyPrinted(obtainedNatspec()) == prettyPrinted(m_expectedNatspecJSON);
}

void NatspecJSONTest::printExpectedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	SyntaxTest::printExpectedResult(_stream, _linePrefix, _formatted);
	if (!m_expectedNatspecJSON.empty())
	{
		_stream << _linePrefix << "----" << std::endl;
		printPrefixed(_stream, formatNatspecExpectations(m_expectedNatspecJSON), _linePrefix);
	}

}

void NatspecJSONTest::printObtainedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	SyntaxTest::printObtainedResult(_stream, _linePrefix, _formatted);

	NatspecMap natspecJSON = obtainedNatspec();
	if (!natspecJSON.empty())
	{
		_stream << _linePrefix << "----" << std::endl;
		// TODO: Diff both versions and highlight differences.
		// We should have a helper for doing that in newly defined test cases without much effort.
		printPrefixed(_stream, formatNatspecExpectations(natspecJSON), _linePrefix);
	}
}

std::tuple<std::string_view, NatspecJSONKind> NatspecJSONTest::parseExpectationHeader(std::string_view _line)
{
	for (NatspecJSONKind kind: {NatspecJSONKind::Devdoc, NatspecJSONKind::Userdoc})
	{
		std::string kindSuffix = " " + toString(kind);
		if (boost::algorithm::ends_with(_line, kindSuffix))
			return {_line.substr(0, _line.size() - kindSuffix.size()), kind};
	}

	BOOST_THROW_EXCEPTION(std::runtime_error(
		"Natspec kind (devdoc/userdoc) not present in the expectation: "s.append(_line)
	));
}

std::string NatspecJSONTest::extractExpectationJSON(std::istream& _stream)
{
	std::string rawJSON;
	std::string line;
	while (getline(_stream, line))
	{
		std::string_view strippedLine = expectLinePrefix(line);
		rawJSON += strippedLine;
		rawJSON += "\n";

		if (boost::algorithm::starts_with(strippedLine, "}"))
			break;
	}

	return rawJSON;
}

std::string_view NatspecJSONTest::expectLinePrefix(std::string_view _line)
{
	size_t startPosition = 0;
	if (!boost::algorithm::starts_with(_line, "//"))
		BOOST_THROW_EXCEPTION(std::runtime_error(
			"Expectation line is not a comment: "s.append(_line)
		));

	startPosition += 2;
	if (startPosition < _line.size() && _line[startPosition] == ' ')
		++startPosition;

	return _line.substr(startPosition, _line.size() - startPosition);
}

std::string NatspecJSONTest::formatNatspecExpectations(NatspecMap const& _expectations) const
{
	std::string output;
	bool first = true;
	// NOTE: Not sorting explicitly because CompilerStack seems to put contracts roughly in the
	// order in which they appear in the source, which is much better than alphabetical order.
	for (auto const& [contractName, expectationsForAllKinds]: _expectations)
		for (auto const& [jsonKind, natspecJSON]: expectationsForAllKinds)
		{
			if (!first)
				output += "\n\n";
			first = false;

			output += contractName + " " + toString(jsonKind) + "\n";
			output += jsonPrint(natspecJSON, {JsonFormat::Pretty, 4});
		}

	return output;
}

NatspecMap NatspecJSONTest::obtainedNatspec() const
{
	if (compiler().state() < CompilerStack::AnalysisSuccessful)
		return {};

	NatspecMap result;
	for (std::string contractName: compiler().contractNames())
	{
		result[contractName][NatspecJSONKind::Devdoc]  = compiler().natspecDev(contractName);
		result[contractName][NatspecJSONKind::Userdoc] = compiler().natspecUser(contractName);
	}

	return result;
}

SerializedNatspecMap NatspecJSONTest::prettyPrinted(NatspecMap const& _expectations) const
{
	SerializedNatspecMap result;
	for (auto const& [contractName, expectationsForAllKinds]: _expectations)
		for (auto const& [jsonKind, natspecJSON]: expectationsForAllKinds)
			result[contractName][jsonKind] = jsonPrint(natspecJSON, {JsonFormat::Pretty, 4});

	return result;
}
