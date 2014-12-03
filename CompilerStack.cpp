/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#include <libsolidity/AST.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/GlobalContext.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/CompilerStack.h>

using namespace std;

namespace dev
{
namespace solidity
{

void CompilerStack::addSource(string const& _name, string const& _content)
{
	if (m_sources.count(_name))
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Source by given name already exists."));

	reset(true);
	m_sources[_name].scanner = make_shared<Scanner>(CharStream(_content), _name);
}

void CompilerStack::setSource(string const& _sourceCode)
{
	reset();
	addSource("", _sourceCode);
}

void CompilerStack::parse()
{
	for (auto& sourcePair: m_sources)
	{
		sourcePair.second.scanner->reset();
		sourcePair.second.ast = Parser().parse(sourcePair.second.scanner);
	}
	resolveImports();

	m_globalContext = make_shared<GlobalContext>();
	NameAndTypeResolver resolver(m_globalContext->getDeclarations());
	for (Source const* source: m_sourceOrder)
		resolver.registerDeclarations(*source->ast);
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				m_globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*m_globalContext->getCurrentThis());
				resolver.resolveNamesAndTypes(*contract);
				m_contracts[contract->getName()].contract = contract;
			}
	m_parseSuccessful = true;
}

void CompilerStack::parse(string const& _sourceCode)
{
	setSource(_sourceCode);
	parse();
}

void CompilerStack::compile(bool _optimize)
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				m_globalContext->setCurrentContract(*contract);
				shared_ptr<Compiler> compiler = make_shared<Compiler>();
				compiler->compileContract(*contract, m_globalContext->getMagicVariables());
				Contract& compiledContract = m_contracts[contract->getName()];
				compiledContract.bytecode = compiler->getAssembledBytecode(_optimize);
				compiledContract.compiler = move(compiler);
			}
}

bytes const& CompilerStack::compile(string const& _sourceCode, bool _optimize)
{
	parse(_sourceCode);
	compile(_optimize);
	return getBytecode();
}

bytes const& CompilerStack::getBytecode(string const& _contractName)
{
	return getContract(_contractName).bytecode;
}

void CompilerStack::streamAssembly(ostream& _outStream, string const& _contractName)
{
	getContract(_contractName).compiler->streamAssembly(_outStream);
}

string const& CompilerStack::getInterface(std::string const& _contractName)
{
	Contract& contract = getContract(_contractName);
	if (contract.interface.empty())
	{
		stringstream interface;
		interface << '[';
		vector<FunctionDefinition const*> exportedFunctions = contract.contract->getInterfaceFunctions();
		unsigned functionsCount = exportedFunctions.size();
		for (FunctionDefinition const* f: exportedFunctions)
		{
			auto streamVariables = [&](vector<ASTPointer<VariableDeclaration>> const& _vars)
			{
				unsigned varCount = _vars.size();
				for (ASTPointer<VariableDeclaration> const& var: _vars)
				{
					interface << "{"
							  << "\"name\":" << escaped(var->getName(), false) << ","
							  << "\"type\":" << escaped(var->getType()->toString(), false)
							  << "}";
					if (--varCount > 0)
						interface << ",";
				}
			};

			interface << '{'
					  << "\"name\":" << escaped(f->getName(), false) << ","
					  << "\"inputs\":[";
			streamVariables(f->getParameters());
			interface << "],"
					  << "\"outputs\":[";
			streamVariables(f->getReturnParameters());
			interface << "]"
					  << "}";
			if (--functionsCount > 0)
				interface << ",";
		}
		interface << ']';
		contract.interface = interface.str();
	}
	return contract.interface;
}

Scanner const& CompilerStack::getScanner(string const& _sourceName)
{
	return *getSource(_sourceName).scanner;
}

SourceUnit& CompilerStack::getAST(string const& _sourceName)
{
	return *getSource(_sourceName).ast;
}

bytes CompilerStack::staticCompile(std::string const& _sourceCode, bool _optimize)
{
	CompilerStack stack;
	return stack.compile(_sourceCode, _optimize);
}

void CompilerStack::reset(bool _keepSources)
{
	m_parseSuccessful = false;
	if (_keepSources)
		for (auto sourcePair: m_sources)
			sourcePair.second.reset();
	else
		m_sources.clear();
	m_globalContext.reset();
	m_compiler.reset();
	m_sourceOrder.clear();
	m_contracts.clear();
}

void CompilerStack::resolveImports()
{
	// topological sorting (depth first search) of the import graph, cutting potential cycles
	vector<Source const*> sourceOrder;
	set<Source const*> sourcesSeen;

	function<void(Source const*)> toposort = [&](Source const* _source)
	{
		if (sourcesSeen.count(_source))
			return;
		sourcesSeen.insert(_source);
		for (ASTPointer<ASTNode> const& node: _source->ast->getNodes())
			if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
			{
				string const& url = import->getURL();
				if (!m_sources.count(url))
					BOOST_THROW_EXCEPTION(ParserError()
										  << errinfo_sourceLocation(import->getLocation())
										  << errinfo_comment("Source not found."));
				toposort(&m_sources[url]);
			}
		sourceOrder.push_back(_source);
	};

	for (auto const& sourcePair: m_sources)
		toposort(&sourcePair.second);

	swap(m_sourceOrder, sourceOrder);
}

CompilerStack::Contract& CompilerStack::getContract(string const& _contractName)
{
	if (m_contracts.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));
	if (_contractName.empty())
		return m_contracts.begin()->second;
	auto it = m_contracts.find(_contractName);
	if (it == m_contracts.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Contract " + _contractName + " not found."));
	return it->second;
}

CompilerStack::Source& CompilerStack::getSource(string const& _sourceName)
{
	auto it = m_sources.find(_sourceName);
	if (it == m_sources.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Given source file not found."));
	return it->second;
}
}
}
