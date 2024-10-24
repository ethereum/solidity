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

#include <test/libsolidity/MemoryGuardTest.h>

#include <test/Common.h>
#include <test/libyul/Common.h>
#include <libsolidity/codegen/ir/Common.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/StringUtils.h>
#include <libyul/Object.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/AST.h>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::test;
using namespace yul;

void MemoryGuardTest::setupCompiler(CompilerStack& _compiler)
{
	AnalysisFramework::setupCompiler(_compiler);

	_compiler.setViaIR(true);
	_compiler.setOptimiserSettings(OptimiserSettings::none());
}

TestCase::TestResult MemoryGuardTest::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	if (!runFramework(m_source, PipelineStage::Compilation))
	{
		printPrefixed(_stream, formatErrors(filteredErrors(), _formatted), _linePrefix);
		return TestResult::FatalError;
	}

	m_obtainedResult.clear();
	for (std::string contractName: compiler().contractNames())
	{
		auto const& dialect = EVMDialect::strictAssemblyForEVMObjects(CommonOptions::get().evmVersion(), CommonOptions::get().eofVersion());
		ErrorList errors;
		std::optional<std::string> const& ir = compiler().yulIR(contractName);
		solAssert(ir);
		auto [object, analysisInfo] = yul::test::parse(
			*ir,
			dialect,
			errors
		);

		if (!object || !analysisInfo || Error::containsErrors(errors))
		{
			AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing IR:" << std::endl;
			printPrefixed(_stream, formatErrors(filterErrors(errors), _formatted), _linePrefix);
			return TestResult::FatalError;
		}

		auto handleObject = [&](std::string const& _kind, Object const& _object) {
			m_obtainedResult += contractName + "(" + _kind + ") " + (findFunctionCalls(
				_object.code()->root(),
				"memoryguard",
				dialect
			).empty() ? "false" : "true") + "\n";
		};
		handleObject("creation", *object);
		size_t deployedIndex = object->subIndexByName.at(
			IRNames::deployedObject(compiler().contractDefinition(contractName))
		);
		handleObject("runtime", dynamic_cast<Object const&>(*object->subObjects[deployedIndex]));
	}
	return checkResult(_stream, _linePrefix, _formatted);
}
