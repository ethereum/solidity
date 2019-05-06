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

#include <test/libsolidity/GasTest.h>
#include <test/Options.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/JSON.h>
#include <liblangutil/SourceReferenceFormatterHuman.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <stdexcept>

using namespace langutil;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace dev;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;


namespace
{

u256 parseGasCost(string::iterator& _it, string::iterator _end)
{
	if (_it == _end || !isdigit(*_it))
		throw runtime_error("Invalid test expectation: expected gas cost.");
	auto begin = _it;
	while (_it != _end && isdigit(*_it))
		++_it;
	return u256(std::string(begin, _it));
}

}

GasTest::GasTest(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);

	if (m_settings.count("optimize"))
	{
		m_optimise = true;
		m_validatedSettings["optimize"] = "true";
		m_settings.erase("optimize");
	}
	if (m_settings.count("optimize-yul"))
	{
		m_optimiseYul = true;
		m_validatedSettings["optimize-yul"] = "true";
		m_settings.erase("optimize-yul");
	}
	if (m_settings.count("optimize-runs"))
	{
		m_optimiseRuns = stoul(m_settings["optimize-runs"]);
		m_validatedSettings["optimize-runs"] = m_settings["optimize-runs"];
		m_settings.erase("optimize-runs");
	}

	parseExpectations(file);
}

void GasTest::parseExpectations(std::istream& _stream)
{
	std::map<std::string, std::string>* currentKind = nullptr;
	std::string line;

	while (getline(_stream, line))
		if (boost::starts_with(line, "// creation:"))
		{
			auto it = line.begin() + 12;
			skipWhitespace(it, line.end());
			m_creationCost.executionCost = parseGasCost(it, line.end());
			skipWhitespace(it, line.end());
			if (*it++ != '+')
				BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected \"+\"-"));
			skipWhitespace(it, line.end());
			m_creationCost.codeDepositCost = parseGasCost(it, line.end());
			skipWhitespace(it, line.end());
			if (*it++ != '=')
				BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected \"+\"-"));
			skipWhitespace(it, line.end());
			m_creationCost.totalCost = parseGasCost(it, line.end());
		}
		else if (line == "// external:")
			currentKind = &m_externalFunctionCosts;
		else if (line == "// internal:")
			currentKind = &m_internalFunctionCosts;
		else if (!currentKind)
			BOOST_THROW_EXCEPTION(runtime_error("No function kind specified. Expected \"external:\" or \"internal:\"."));
		else
		{
			if (!boost::starts_with(line, "// "))
				BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected \"// \"."));
			auto it = line.begin() + 3;
			skipWhitespace(it, line.end());
			auto functionNameBegin = it;
			while (it != line.end() && *it != ':')
				++it;
			std::string functionName(functionNameBegin, it);
			if (functionName == "fallback")
				functionName.clear();
			expect(it, line.end(), ':');
			skipWhitespace(it, line.end());
			if (it == line.end())
				BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected gas cost."));
			(*currentKind)[functionName] = std::string(it, line.end());
		}
}

void GasTest::printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const
{
	Json::Value estimates = compiler().gasEstimates(compiler().lastContractName());
	_stream << _linePrefix
		<< "creation: "
		<< estimates["creation"]["executionCost"].asString()
		<< " + "
		<< estimates["creation"]["codeDepositCost"].asString()
		<< " = "
		<< estimates["creation"]["totalCost"].asString()
		<< std::endl;

	for (auto kind: {"external", "internal"})
		if (estimates[kind])
		{
			_stream << _linePrefix << kind << ":" << std::endl;
			for (auto it = estimates[kind].begin(); it != estimates[kind].end(); ++it)
			{
				_stream << _linePrefix << "  ";
				if (it.key().asString().empty())
					_stream << "fallback";
				else
					_stream << it.key().asString();
				_stream << ": " << it->asString() << std::endl;
			}
		}
}


bool GasTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	string const versionPragma = "pragma solidity >=0.0;\n";
	compiler().reset();
	OptimiserSettings settings = m_optimise ? OptimiserSettings::standard() : OptimiserSettings::minimal();
	if (m_optimiseYul)
	{
		settings.runYulOptimiser = m_optimise;
		settings.optimizeStackAllocation = m_optimise;
	}
	settings.expectedExecutionsPerDeployment = m_optimiseRuns;
	compiler().setOptimiserSettings(settings);
	compiler().setSources({{"", versionPragma + m_source}});

	if (!compiler().parseAndAnalyze() || !compiler().compile())
	{
		SourceReferenceFormatterHuman formatter(cerr, _formatted);
		for (auto const& error: compiler().errors())
			formatter.printErrorInformation(*error);
		BOOST_THROW_EXCEPTION(runtime_error("Test contract does not compile."));
	}

	Json::Value estimates = compiler().gasEstimates(compiler().lastContractName());


	auto creation = estimates["creation"];
	bool success =
		(creation["codeDepositCost"].asString() == toString(m_creationCost.codeDepositCost)) &&
		(creation["executionCost"].asString() == toString(m_creationCost.executionCost)) &&
		(creation["totalCost"].asString() == toString(m_creationCost.totalCost));

	auto check = [&](map<string, string> const& _a, Json::Value const& _b) {
		for (auto& entry: _a)
			success &= _b[entry.first].asString() == entry.second;
	};
	check(m_internalFunctionCosts, estimates["internal"]);
	check(m_externalFunctionCosts, estimates["external"]);

	if (!success)
	{
		_stream << _linePrefix << "Expected:" << std::endl;
		_stream << _linePrefix
			<< "  creation: "
			<< toString(m_creationCost.executionCost)
			<< " + "
			<< toString(m_creationCost.codeDepositCost)
			<< " = "
			<< toString(m_creationCost.totalCost)
			<< std::endl;
		auto printExpected = [&](std::string const& _kind, auto const& _expectations)
		{
			_stream << _linePrefix << "  " << _kind << ":" << std::endl;
			for (auto const& entry: _expectations)
				_stream << _linePrefix
					<< "    "
					<< (entry.first.empty() ? "fallback" : entry.first)
					<< ": "
					<< entry.second
					<< std::endl;
		};
		if (!m_externalFunctionCosts.empty())
			printExpected("external", m_externalFunctionCosts);
		if (!m_internalFunctionCosts.empty())
			printExpected("internal", m_internalFunctionCosts);
		_stream << _linePrefix << "Obtained:" << std::endl;
		printUpdatedExpectations(_stream, _linePrefix + "  ");
	}

	return success;
}

void GasTest::printSource(ostream& _stream, string const& _linePrefix, bool) const
{
	std::string line;
	std::istringstream input(m_source);
	while (getline(input, line))
		_stream << _linePrefix << line << std::endl;
}
