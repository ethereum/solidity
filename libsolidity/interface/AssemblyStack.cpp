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
	m_analysisSuccessful = false;
	m_scanner = make_shared<Scanner>(CharStream(_source), _sourceName);
	m_parserResult = assembly::Parser(m_errors, m_language == Language::JULIA).parse(m_scanner);
	if (!m_errors.empty())
		return false;
	solAssert(m_parserResult, "");

	m_analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
	assembly::AsmAnalyzer analyzer(*m_analysisInfo, m_errors);
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
		auto assembly = assembly::CodeGenerator(m_errors).assemble(*m_parserResult, *m_analysisInfo);
		return assembly.assemble();
	}
	case Machine::EVM15:
		solUnimplemented("EVM 1.5 backend is not yet implemented.");
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
