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

#include <jsonrpc/json/json.h>

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

string const& CompilerStack::getInterface()
{
	Json::StyledWriter writer;
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	if (m_interface.empty())
	{
		Json::Value methods(Json::arrayValue);

		vector<FunctionDefinition const*> exportedFunctions = m_contractASTNode->getInterfaceFunctions();
		for (FunctionDefinition const* f: exportedFunctions)
		{
			Json::Value method;
			Json::Value inputs(Json::arrayValue);
			Json::Value outputs(Json::arrayValue);

			auto streamVariables = [](vector<ASTPointer<VariableDeclaration>> const& _vars)
			{
				Json::Value params(Json::arrayValue);
				for (ASTPointer<VariableDeclaration> const& var: _vars)
				{
					Json::Value input;
					input["name"] = var->getName();
					input["type"] = var->getType()->toString();
					params.append(input);
				}
				return params;
			};

			method["name"] = f->getName();
			method["inputs"] = streamVariables(f->getParameters());
			method["outputs"] = streamVariables(f->getReturnParameters());
			methods.append(method);
		}
		m_interface = writer.write(methods);
	}
	return m_interface;
}

string const& CompilerStack::getDocumentation()
{
	Json::StyledWriter writer;
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	if (m_documentation.empty())
	{
		Json::Value doc;
		Json::Value methods(Json::objectValue);

		for (FunctionDefinition const* f: m_contractASTNode->getInterfaceFunctions())
		{
			Json::Value user;
			auto strPtr = f->getDocumentation();
			if (strPtr)
			{
				user["user"] = Json::Value(*strPtr);
				methods[f->getName()] = user;
			}
		}
		doc["methods"] = methods;
		m_documentation = writer.write(doc);
	}
	return m_documentation;
}

bytes CompilerStack::staticCompile(std::string const& _sourceCode, bool _optimize)
{
	CompilerStack stack;
	return stack.compile(_sourceCode, _optimize);
}



}
}
