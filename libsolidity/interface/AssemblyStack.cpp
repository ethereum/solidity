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
#include <libsolidity/inlineasm/AsmCodeGen.h>

#include <libevmasm/Assembly.h>

#include <libjulia/backends/evm/EVMCodeTransform.h>
#include <libjulia/backends/evm/EVMAssembly.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


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
	m_parserResult = assembly::Parser(m_errorReporter, m_language == Language::JULIA).parse(m_scanner);
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
	assembly::AsmAnalyzer analyzer(*m_analysisInfo, m_errorReporter);
	m_analysisSuccessful = analyzer.analyze(*m_parserResult);
	return m_analysisSuccessful;
}

eth::LinkerObject AssemblyStack::assemble(Machine _machine)
{
	solAssert(m_analysisSuccessful, "");
	solAssert(m_parserResult, "");
	solAssert(m_analysisInfo, "");

	switch (_machine)
	{
	case Machine::EVM:
	{
		auto assembly = assembly::CodeGenerator(m_errorReporter).assemble(*m_parserResult, *m_analysisInfo);
		return assembly.assemble();
	}
	case Machine::EVM15:
	{
		julia::EVMAssembly assembly(true);
		julia::CodeTransform(m_errorReporter, assembly, *m_analysisInfo, true).run(*m_parserResult);
		return assembly.finalize();
	}
	case Machine::eWasm:
		solUnimplemented("eWasm backend is not yet implemented.");
	}
	// unreachable
	return eth::LinkerObject();
}

string AssemblyStack::print()
{
	solAssert(m_parserResult, "");
	return assembly::AsmPrinter(m_language == Language::JULIA)(*m_parserResult);
}
