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

namespace
{
std::string toString(SideEffects const& _sideEffects)
{
	std::vector<std::string> ret;
	if (_sideEffects.movable)
		ret.emplace_back("movable");
	if (_sideEffects.movableApartFromEffects)
		ret.emplace_back("movable apart from effects");
	if (_sideEffects.canBeRemoved)
		ret.emplace_back("can be removed");
	if (_sideEffects.canBeRemovedIfNoMSize)
		ret.emplace_back("can be removed if no msize");
	if (!_sideEffects.cannotLoop)
		ret.emplace_back("can loop");
	if (_sideEffects.otherState == SideEffects::Write)
		ret.emplace_back("writes other state");
	else if (_sideEffects.otherState == SideEffects::Read)
		ret.emplace_back("reads other state");
	if (_sideEffects.storage == SideEffects::Write)
		ret.emplace_back("writes storage");
	else if (_sideEffects.storage == SideEffects::Read)
		ret.emplace_back("reads storage");
	if (_sideEffects.memory == SideEffects::Write)
		ret.emplace_back("writes memory");
	else if (_sideEffects.memory == SideEffects::Read)
		ret.emplace_back("reads memory");

	return joinHumanReadable(ret);
}
}

FunctionSideEffects::FunctionSideEffects(std::string const& _filename):
	TestCase(_filename)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult FunctionSideEffects::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVMObjects(
		solidity::test::CommonOptions::get().evmVersion(),
		solidity::test::CommonOptions::get().eofVersion()
	);
	Object obj{dialect};
	auto parsingResult = yul::test::parse(m_source);
	obj.setCode(parsingResult.first, parsingResult.second);
	if (!obj.hasCode())
		BOOST_THROW_EXCEPTION(std::runtime_error("Parsing input failed."));

	std::map<YulName, SideEffects> functionSideEffects = SideEffectsPropagator::sideEffects(
		dialect,
		CallGraphGenerator::callGraph(obj.code()->root())
	);

	std::map<std::string, std::string> functionSideEffectsStr;
	for (auto const& fun: functionSideEffects)
		functionSideEffectsStr[fun.first.str()] = toString(fun.second);

	m_obtainedResult.clear();
	for (auto const& fun: functionSideEffectsStr)
		m_obtainedResult += fun.first + ":" + (fun.second.empty() ? "" : " ") + fun.second + "\n";

	return checkResult(_stream, _linePrefix, _formatted);
}
