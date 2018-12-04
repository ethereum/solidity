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
 * Full assembly stack that can support EVM-assembly and Yul as input and EVM, EVM1.5 and
 * eWasm as output.
 */


#include <libsolidity/interface/AssemblyStack.h>

#include <liblangutil/Scanner.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmCodeGen.h>
#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMAssembly.h>
#include <libyul/ObjectParser.h>

#include <libevmasm/Assembly.h>

#include <libyul/optimiser/Suite.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

namespace
{
yul::Dialect languageToDialect(AssemblyStack::Language _language)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return yul::Dialect::looseAssemblyForEVM();
	case AssemblyStack::Language::StrictAssembly:
		return yul::Dialect::strictAssemblyForEVMObjects();
	case AssemblyStack::Language::Yul:
		return yul::Dialect::yul();
	}
	solAssert(false, "");
	return yul::Dialect::yul();
}

}


Scanner const& AssemblyStack::scanner() const
{
	solAssert(m_scanner, "");
	return *m_scanner;
}

bool AssemblyStack::parseAndAnalyze(std::string const& _sourceName, std::string const& _source)
{
	m_errors.clear();
	m_analysisSuccessful = false;
	m_scanner = make_shared<Scanner>(CharStream(_source, _sourceName));
	m_parserResult = yul::ObjectParser(m_errorReporter, languageToDialect(m_language)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");

	return analyzeParsed();
}

void AssemblyStack::optimize()
{
	solAssert(m_language != Language::Assembly, "Optimization requested for loose assembly.");
	yul::OptimiserSuite::run(*m_parserResult->code, *m_parserResult->analysisInfo);
	solAssert(analyzeParsed(), "Invalid source code after optimization.");
}

bool AssemblyStack::analyzeParsed()
{
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");
	m_parserResult->analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(*m_parserResult->analysisInfo, m_errorReporter, m_evmVersion, boost::none, languageToDialect(m_language));
	m_analysisSuccessful = analyzer.analyze(*m_parserResult->code);
	return m_analysisSuccessful;
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine) const
{
	solAssert(m_analysisSuccessful, "");
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");
	solAssert(m_parserResult->analysisInfo, "");

	switch (_machine)
	{
	case Machine::EVM:
	{
		MachineAssemblyObject object;
		eth::Assembly assembly;
		yul::CodeGenerator::assemble(*m_parserResult->code, *m_parserResult->analysisInfo, assembly);
		object.bytecode = make_shared<eth::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::EVM15:
	{
		MachineAssemblyObject object;
		yul::EVMAssembly assembly(true);
		yul::CodeTransform(assembly, *m_parserResult->analysisInfo, m_language == Language::Yul, true)(*m_parserResult->code);
		object.bytecode = make_shared<eth::LinkerObject>(assembly.finalize());
		/// TODO: fill out text representation
		return object;
	}
	case Machine::eWasm:
		solUnimplemented("eWasm backend is not yet implemented.");
	}
	// unreachable
	return MachineAssemblyObject();
}

string AssemblyStack::print() const
{
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");
	return m_parserResult->toString(m_language == Language::Yul) + "\n";
}
