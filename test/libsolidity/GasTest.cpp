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
	map<std::string, std::string>* currentKind = nullptr;
	string line;

	while (getline(_stream, line))
		if (!boost::starts_with(line, "// "))
			BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected \"// \"."));
		else if (boost::ends_with(line, ":"))
		{
			string kind = line.substr(3, line.length() - 4);
			boost::trim(kind);
			currentKind = &m_expectations[move(kind)];
		}
		else if (!currentKind)
			BOOST_THROW_EXCEPTION(runtime_error("No function kind specified. Expected \"creation:\", \"external:\" or \"internal:\"."));
		else
		{
			auto it = line.begin() + 3;
			skipWhitespace(it, line.end());
			auto functionNameBegin = it;
			while (it != line.end() && *it != ':')
				++it;
			string functionName(functionNameBegin, it);
			if (functionName == "fallback")
				functionName.clear();
			expect(it, line.end(), ':');
			skipWhitespace(it, line.end());
			if (it == line.end())
				BOOST_THROW_EXCEPTION(runtime_error("Invalid expectation: expected gas cost."));
			(*currentKind)[functionName] = std::string(it, line.end());
		}
}

void GasTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	Json::Value estimates = compiler().gasEstimates(compiler().lastContractName());
	for (auto groupIt = estimates.begin(); groupIt != estimates.end(); ++groupIt)
	{
		_stream << _linePrefix << groupIt.key().asString() << ":" << std::endl;
		for (auto it = groupIt->begin(); it != groupIt->end(); ++it)
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

TestCase::TestResult GasTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	string const versionPragma = "pragma solidity >=0.0;\n";
	compiler().reset();
	// Prerelease CBOR metadata varies in size due to changing version numbers and build dates.
	// This leads to volatile creation cost estimates. Therefore we force the compiler to
	// release mode for testing gas estimates.
	compiler().overwriteReleaseFlag(true);
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
		SourceReferenceFormatterHuman formatter(_stream, _formatted);
		for (auto const& error: compiler().errors())
			formatter.printErrorInformation(*error);
		return TestResult::FatalError;
	}

	Json::Value estimateGroups = compiler().gasEstimates(compiler().lastContractName());
	if (
		m_expectations.size() == estimateGroups.size() &&
		boost::all(m_expectations, [&](auto const& expectations) {
		auto const& estimates = estimateGroups[expectations.first];
		return estimates.size() == expectations.second.size() &&
			boost::all(expectations.second, [&](auto const& entry) {
				return entry.second == estimates[entry.first].asString();
			});
		})
	)
		return TestResult::Success;
	else
	{
		_stream << _linePrefix << "Expected:" << std::endl;
		for (auto const& expectations: m_expectations)
		{
			_stream << _linePrefix << "  " << expectations.first << ":" << std::endl;
			for (auto const& entry: expectations.second)
				_stream << _linePrefix
					<< "    "
					<< (entry.first.empty() ? "fallback" : entry.first)
					<< ": "
					<< entry.second
					<< std::endl;
		}
		_stream << _linePrefix << "Obtained:" << std::endl;
		printUpdatedExpectations(_stream, _linePrefix + "  ");
		return TestResult::Failure;
	}
}

void GasTest::printSource(ostream& _stream, string const& _linePrefix, bool) const
{
	string line;
	istringstream input(m_source);
	while (getline(input, line))
		_stream << _linePrefix << line << std::endl;
}
