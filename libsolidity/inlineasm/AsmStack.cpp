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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Full-stack Solidity inline assember.
 */

#include <libsolidity/inlineasm/AsmStack.h>

#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmCodeGen.h>
#include <libsolidity/inlineasm/AsmPrinter.h>
#include <libsolidity/inlineasm/AsmDesugar.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>

#include <libsolidity/parsing/Scanner.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>

#include <memory>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

bool InlineAssemblyStack::parse(shared_ptr<Scanner> const& _scanner)
{
	m_parserResult = make_shared<Block>();
	Parser parser(m_errors);
	auto result = parser.parse(_scanner);
	if (!result)
		return false;

	*m_parserResult = std::move(*result);
	m_scopes.clear();
	return (AsmAnalyzer(m_scopes, m_errors, false)).analyze(*m_parserResult);
}

string InlineAssemblyStack::toString()
{
	return AsmPrinter()(*m_parserResult);
}

eth::Assembly InlineAssemblyStack::assemble()
{
	if (!m_desugared)
		desugar();
	CodeGenerator codeGen(*m_parserResult, m_errors);
	return codeGen.assemble();
}

void InlineAssemblyStack::desugar()
{
	solAssert(!m_scopes.empty(), "");
	*m_parserResult = AsmDesugar(m_scopes).run(*m_parserResult);
	// Pointing to invalid data since parserResult is overwritten.
	m_scopes.clear();
	m_desugared = true;
}

bool InlineAssemblyStack::parseAndAssemble(
	string const& _input,
	eth::Assembly& _assembly,
	CodeGenerator::IdentifierAccess const& _identifierAccess
)
{
	auto scanner = make_shared<Scanner>(CharStream(_input), "--CODEGEN--");
	parse(scanner);
	if (!m_errors.empty())
		return false;
	// @todo make this optional in the future.
	desugar();

	CodeGenerator(*m_parserResult, m_errors).assemble(_assembly, _identifierAccess);

	// If there is an error, the assembly might be messed up, but we should throw an
	// internal compiler error anyway.
	return m_errors.empty();
}

