// SPDX-License-Identifier: GPL-3.0

#include <test/libyul/FunctionSideEffects.h>
#include <test/Common.h>
#include <test/libyul/Common.h>

#include <libsolutil/AnsiColorized.h>

#include <libyul/SideEffects.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/Object.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>


using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

namespace
{
string toString(SideEffects const& _sideEffects)
{
	vector<string> ret;
	if (_sideEffects.movable)
		ret.emplace_back("movable");
	if (_sideEffects.sideEffectFree)
		ret.emplace_back("sideEffectFree");
	if (_sideEffects.sideEffectFreeIfNoMSize)
		ret.emplace_back("sideEffectFreeIfNoMSize");
	if (_sideEffects.invalidatesStorage)
		ret.emplace_back("invalidatesStorage");
	if (_sideEffects.invalidatesMemory)
		ret.emplace_back("invalidatesMemory");
	return joinHumanReadable(ret);
}
}

FunctionSideEffects::FunctionSideEffects(string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

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
		m_obtainedResult += fun.first + ":" + (fun.second.empty() ? "" : " ") + fun.second + "\n";

	return checkResult(_stream, _linePrefix, _formatted);
}
