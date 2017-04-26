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
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>

#include <libsolidity/parsing/Scanner.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>

#include <memory>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

bool InlineAssemblyStack::parse(
	shared_ptr<Scanner> const& _scanner,
	ExternalIdentifierAccess::Resolver const& _resolver
)
{
	m_parserResult = make_shared<Block>();
	Parser parser(m_errors);
	auto result = parser.parse(_scanner);
	if (!result)
		return false;

	*m_parserResult = std::move(*result);
	AsmAnalysisInfo analysisInfo;
	return (AsmAnalyzer(analysisInfo, m_errors, _resolver)).analyze(*m_parserResult);
}

string InlineAssemblyStack::toString()
{
	return AsmPrinter()(*m_parserResult);
}

eth::Assembly InlineAssemblyStack::assemble()
{
	AsmAnalysisInfo analysisInfo;
	AsmAnalyzer analyzer(analysisInfo, m_errors);
	solAssert(analyzer.analyze(*m_parserResult), "");
	CodeGenerator codeGen(m_errors);
	return codeGen.assemble(*m_parserResult, analysisInfo);
}

bool InlineAssemblyStack::parseAndAssemble(
	string const& _input,
	eth::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	ErrorList errors;
	auto scanner = make_shared<Scanner>(CharStream(_input), "--CODEGEN--");
	auto parserResult = Parser(errors).parse(scanner);
	if (!errors.empty())
		return false;
	solAssert(parserResult, "");

	AsmAnalysisInfo analysisInfo;
	AsmAnalyzer analyzer(analysisInfo, errors, _identifierAccess.resolve);
	solAssert(analyzer.analyze(*parserResult), "");
	CodeGenerator(errors).assemble(*parserResult, analysisInfo, _assembly, _identifierAccess);

	// At this point, the assembly might be messed up, but we should throw an
	// internal compiler error anyway.
	return errors.empty();
}

