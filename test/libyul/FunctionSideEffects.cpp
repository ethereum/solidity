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

#include <test/libyul/FunctionSideEffects.h>
#include <test/Options.h>
#include <test/libyul/Common.h>

#include <libdevcore/AnsiColorized.h>

#include <libyul/SideEffects.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/Object.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/string.hpp>


using namespace dev;
using namespace langutil;
using namespace yul;
using namespace yul::test;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;

namespace
{
string toString(SideEffects const& _sideEffects)
{
	vector<string> ret;
	if (_sideEffects.movable)
		ret.push_back("movable");
	if (_sideEffects.sideEffectFree)
		ret.push_back("sideEffectFree");
	if (_sideEffects.sideEffectFreeIfNoMSize)
		ret.push_back("sideEffectFreeIfNoMSize");
	if (_sideEffects.invalidatesStorage)
		ret.push_back("invalidatesStorage");
	if (_sideEffects.invalidatesMemory)
		ret.push_back("invalidatesMemory");
	return joinHumanReadable(ret);
}
}

FunctionSideEffects::FunctionSideEffects(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test input: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);
	m_expectation = parseSimpleExpectations(file);}

TestCase::TestResult FunctionSideEffects::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	Object obj;
	std::tie(obj.code, obj.analysisInfo) = yul::test::parse(m_source, false);
	if (!obj.code)
		BOOST_THROW_EXCEPTION(runtime_error("Parsing input failed."));

	map<YulString, SideEffects> functionSideEffects = SideEffectsPropagator::sideEffects(
		EVMDialect::strictAssemblyForEVM(langutil::EVMVersion()),
		CallGraphGenerator::callGraph(*obj.code)
	);

	std::map<std::string, std::string> functionSideEffectsStr;
	for (auto const& fun: functionSideEffects)
		functionSideEffectsStr[fun.first.str()] = toString(fun.second);

	m_obtainedResult.clear();
	for (auto const& fun: functionSideEffectsStr)
		m_obtainedResult += fun.first + ": " + fun.second + "\n";

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return TestResult::Failure;
	}
	return TestResult::Success;
}


void FunctionSideEffects::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void FunctionSideEffects::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void FunctionSideEffects::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		if (line.empty())
			// Avoid trailing spaces.
			_stream << boost::trim_right_copy(_linePrefix) << endl;
		else
			_stream << _linePrefix << line << endl;
}
