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

#include <test/libyul/Common.h>
#include <libsolidity/codegen/ir/Common.h>
#include <libsolutil/Algorithms.h>
#include <libyul/Object.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace yul;

TestCase::TestResult MemoryGuardTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	compiler().reset();
	compiler().setSources(StringMap{{"", m_source}});
	compiler().setViaIR(true);
	compiler().setOptimiserSettings(OptimiserSettings::none());
	if (!compiler().compile())
		return TestResult::FatalError;

	m_obtainedResult.clear();
	for (string contractName: compiler().contractNames())
	{
		ErrorList errors;
		auto [object, analysisInfo] = yul::test::parse(
			compiler().yulIR(contractName),
			EVMDialect::strictAssemblyForEVMObjects({}),
			errors
		);

		if (!object || !analysisInfo || Error::containsErrors(errors))
		{
			AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing IR." << endl;
			return TestResult::FatalError;
		}

		auto handleObject = [&](std::string const& _kind, Object const& _object) {
			m_obtainedResult += contractName + "(" + _kind + ") " + (FunctionCallFinder::run(
				*_object.code,
				"memoryguard"_yulstring
			).empty() ? "false" : "true") + "\n";
		};
		handleObject("creation", *object);
		size_t deployedIndex = object->subIndexByName.at(
			YulString(IRNames::deployedObject(compiler().contractDefinition(contractName)))
		);
		handleObject("runtime", dynamic_cast<Object const&>(*object->subObjects[deployedIndex]));
	}
	return checkResult(_stream, _linePrefix, _formatted);
}
