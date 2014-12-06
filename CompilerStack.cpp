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
#include <libsolidity/InterfaceHandler.h>

using namespace std;

namespace dev
{
namespace solidity
{

CompilerStack::CompilerStack(): m_interfaceHandler(make_shared<InterfaceHandler>()) {}

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
	m_globalContext = make_shared<GlobalContext>();
	m_globalContext->setCurrentContract(*m_contractASTNode);
	NameAndTypeResolver(m_globalContext->getDeclarations()).resolveNamesAndTypes(*m_contractASTNode);
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
	m_compiler->compileContract(*m_contractASTNode, m_globalContext->getMagicVariables());
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

std::string const& CompilerStack::getJsonDocumentation(DocumentationType _type)
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	auto createDocIfNotThere = [this, _type](std::unique_ptr<string>& _doc)
	{
		if (!_doc)
			_doc = m_interfaceHandler->getDocumentation(m_contractASTNode, _type);
	};

	switch (_type)
	{
	case DocumentationType::NATSPEC_USER:
		createDocIfNotThere(m_userDocumentation);
		return *m_userDocumentation;
	case DocumentationType::NATSPEC_DEV:
		createDocIfNotThere(m_devDocumentation);
		return *m_devDocumentation;
	case DocumentationType::ABI_INTERFACE:
		createDocIfNotThere(m_interface);
		return *m_interface;
	}

	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Illegal documentation type."));
}

bytes CompilerStack::staticCompile(std::string const& _sourceCode, bool _optimize)
{
	CompilerStack stack;
	return stack.compile(_sourceCode, _optimize);
}



}
}
