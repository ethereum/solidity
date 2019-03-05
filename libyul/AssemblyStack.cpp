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


#include <libyul/AssemblyStack.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/backends/evm/AsmCodeGen.h>
#include <libyul/backends/evm/EVMAssembly.h>
#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMObjectCompiler.h>
#include <libyul/ObjectParser.h>
#include <libyul/optimiser/Suite.h>

#include <libevmasm/Assembly.h>
#include <liblangutil/Scanner.h>

using namespace std;
using namespace langutil;
using namespace yul;

namespace
{
shared_ptr<Dialect> languageToDialect(AssemblyStack::Language _language, EVMVersion _version)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return EVMDialect::looseAssemblyForEVM(_version);
	case AssemblyStack::Language::StrictAssembly:
		return EVMDialect::strictAssemblyForEVMObjects(_version);
	case AssemblyStack::Language::Yul:
		return Dialect::yul();
	}
	solAssert(false, "");
	return Dialect::yul();
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
	m_parserResult = ObjectParser(m_errorReporter, languageToDialect(m_language, m_evmVersion)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");

	return analyzeParsed();
}

void AssemblyStack::optimize()
{
	if (m_language != Language::StrictAssembly)
		solUnimplemented("Optimizer for both loose assembly and Yul is not yet implemented");
	solAssert(m_analysisSuccessful, "Analysis was not successful.");
	m_analysisSuccessful = false;
	solAssert(m_parserResult, "");
	optimize(*m_parserResult);
	solAssert(analyzeParsed(), "Invalid source code after optimization.");
}

bool AssemblyStack::analyzeParsed()
{
	solAssert(m_parserResult, "");
	m_analysisSuccessful = analyzeParsed(*m_parserResult);
	return m_analysisSuccessful;
}

bool AssemblyStack::analyzeParsed(Object& _object)
{
	solAssert(_object.code, "");
	_object.analysisInfo = make_shared<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*_object.analysisInfo, m_errorReporter, boost::none, languageToDialect(m_language, m_evmVersion));
	bool success = analyzer.analyze(*_object.code);
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
			if (!analyzeParsed(*subObject))
				success = false;
	return success;
}

void AssemblyStack::compileEVM(AbstractAssembly& _assembly, bool _evm15, bool _optimize) const
{
	shared_ptr<EVMDialect> dialect;

	if (m_language == Language::Assembly)
		dialect = EVMDialect::looseAssemblyForEVM(m_evmVersion);
	else if (m_language == AssemblyStack::Language::StrictAssembly)
		dialect = EVMDialect::strictAssemblyForEVMObjects(m_evmVersion);
	else if (m_language == AssemblyStack::Language::Yul)
		dialect = EVMDialect::yulForEVM(m_evmVersion);
	else
		solAssert(false, "Invalid language.");

	EVMObjectCompiler::compile(*m_parserResult, _assembly, *dialect, _evm15, _optimize);
}

void AssemblyStack::optimize(Object& _object)
{
	solAssert(_object.code, "");
	solAssert(_object.analysisInfo, "");
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
			optimize(*subObject);
	OptimiserSuite::run(languageToDialect(m_language, m_evmVersion), *_object.code, *_object.analysisInfo);
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine, bool _optimize) const
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
		dev::eth::Assembly assembly;
		EthAssemblyAdapter adapter(assembly);
		compileEVM(adapter, false, _optimize);
		object.bytecode = make_shared<dev::eth::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::EVM15:
	{
		MachineAssemblyObject object;
		EVMAssembly assembly(true);
		compileEVM(assembly, true, _optimize);
		object.bytecode = make_shared<dev::eth::LinkerObject>(assembly.finalize());
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

shared_ptr<Object> AssemblyStack::parserResult() const
{
	solAssert(m_analysisSuccessful, "Analysis was not successful.");
	solAssert(m_parserResult, "");
	solAssert(m_parserResult->code, "");
	return m_parserResult;
}
