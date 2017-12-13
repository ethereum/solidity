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
 * Full assembly stack that can support EVM-assembly and JULIA as input and EVM, EVM1.5 and
 * eWasm as output.
 */


#include <libsolidity/interface/AssemblyStack.h>

#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/inlineasm/AsmPrinter.h>
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmCodeGen.h>

#include <libevmasm/Assembly.h>

#include <libjulia/backends/evm/EVMCodeTransform.h>
#include <libjulia/backends/evm/EVMAssembly.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace
{
assembly::AsmFlavour languageToAsmFlavour(AssemblyStack::Language _language)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return assembly::AsmFlavour::Loose;
	case AssemblyStack::Language::JULIA:
		return assembly::AsmFlavour::IULIA;
	}
	solAssert(false, "");
	return assembly::AsmFlavour::IULIA;
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
	m_scanner = make_shared<Scanner>(CharStream(_source), _sourceName);
	m_parserResult = assembly::Parser(m_errorReporter, languageToAsmFlavour(m_language)).parse(m_scanner);
	if (!m_errorReporter.errors().empty())
		return false;
	solAssert(m_parserResult, "");

	return analyzeParsed();
}

bool AssemblyStack::analyze(assembly::Block const& _block, Scanner const* _scanner)
{
	m_errors.clear();
	m_analysisSuccessful = false;
	if (_scanner)
		m_scanner = make_shared<Scanner>(*_scanner);
	m_parserResult = make_shared<assembly::Block>(_block);

	return analyzeParsed();
}

bool AssemblyStack::analyzeParsed()
{
	m_analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
	assembly::AsmAnalyzer analyzer(*m_analysisInfo, m_errorReporter, languageToAsmFlavour(m_language));
	m_analysisSuccessful = analyzer.analyze(*m_parserResult);
	return m_analysisSuccessful;
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine) const
{
	solAssert(m_analysisSuccessful, "");
	solAssert(m_parserResult, "");
	solAssert(m_analysisInfo, "");

	switch (_machine)
	{
	case Machine::EVM:
	{
		MachineAssemblyObject object;
		eth::Assembly assembly;
		assembly::CodeGenerator::assemble(*m_parserResult, *m_analysisInfo, assembly);
		object.bytecode = make_shared<eth::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::EVM15:
	{
		MachineAssemblyObject object;
		julia::EVMAssembly assembly(true);
		julia::CodeTransform(assembly, *m_analysisInfo, m_language == Language::JULIA, true)(*m_parserResult);
		object.bytecode = make_shared<eth::LinkerObject>(assembly.finalize());
		/// TOOD: fill out text representation
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
	return assembly::AsmPrinter(m_language == Language::JULIA)(*m_parserResult);
}
