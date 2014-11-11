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
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/CompilerStack.h>

using namespace std;

namespace dev
{
namespace solidity
{

void CompilerStack::setSource(string const& _sourceCode)
{
	reset();
	m_scanner = make_shared<Scanner>(CharStream(_sourceCode));
}

void CompilerStack::parse()
{
	if (!m_scanner)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Source not available."));
	m_contractASTNode = Parser().parse(m_scanner);
	NameAndTypeResolver().resolveNamesAndTypes(*m_contractASTNode);
	m_parseSuccessful = true;
}

void CompilerStack::parse(string const& _sourceCode)
{
	setSource(_sourceCode);
	parse();
}

bytes const& CompilerStack::compile(bool _optimize)
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	m_bytecode.clear();
	m_compiler = make_shared<Compiler>();
	m_compiler->compileContract(*m_contractASTNode);
	return m_bytecode = m_compiler->getAssembledBytecode(_optimize);
}

bytes const& CompilerStack::compile(string const& _sourceCode, bool _optimize)
{
	parse(_sourceCode);
	return compile(_optimize);
}

void CompilerStack::streamAssembly(ostream& _outStream)
{
	if (!m_compiler || m_bytecode.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));
	m_compiler->streamAssembly(_outStream);
}

string const& CompilerStack::getInterface()
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	if (m_interface.empty())
	{
		stringstream interface;
		interface << '[';
		vector<FunctionDefinition const*> exportedFunctions = m_contractASTNode->getInterfaceFunctions();
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
		m_interface = interface.str();
	}
	return m_interface;
}

bytes CompilerStack::staticCompile(std::string const& _sourceCode, bool _optimize)
{
	CompilerStack stack;
	return stack.compile(_sourceCode, _optimize);
}



}
}
