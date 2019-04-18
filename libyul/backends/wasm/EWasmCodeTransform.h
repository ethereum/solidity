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
 * Common code generator for translating Yul / inline assembly to EWasm.
 */

#pragma once

#include <libyul/backends/wasm/EWasmAST.h>
#include <libyul/AsmDataForward.h>
#include <libyul/Dialect.h>

#include <stack>

namespace yul
{
struct AsmAnalysisInfo;

class EWasmCodeTransform: public boost::static_visitor<wasm::Expression>
{
public:
	EWasmCodeTransform(
		AsmAnalysisInfo&,
		Dialect const& _dialect
	):
		m_dialect(_dialect)
	{}

	std::string run(yul::Block const& _ast);

public:
	wasm::Expression operator()(yul::Instruction const& _instruction);
	wasm::Expression operator()(yul::Literal const& _literal);
	wasm::Expression operator()(yul::Identifier const& _identifier);
	wasm::Expression operator()(yul::FunctionalInstruction const& _instr);
	wasm::Expression operator()(yul::FunctionCall const&);
	wasm::Expression operator()(yul::ExpressionStatement const& _statement);
	wasm::Expression operator()(yul::Label const& _label);
	wasm::Expression operator()(yul::StackAssignment const& _assignment);
	wasm::Expression operator()(yul::Assignment const& _assignment);
	wasm::Expression operator()(yul::VariableDeclaration const& _varDecl);
	wasm::Expression operator()(yul::If const& _if);
	wasm::Expression operator()(yul::Switch const& _switch);
	wasm::Expression operator()(yul::FunctionDefinition const&);
	wasm::Expression operator()(yul::ForLoop const&);
	wasm::Expression operator()(yul::Break const&);
	wasm::Expression operator()(yul::Continue const&);
	wasm::Expression operator()(yul::Block const& _block);

private:
	std::unique_ptr<wasm::Expression> visit(yul::Expression const& _expression);
	wasm::Expression visitReturnByValue(yul::Expression const& _expression);
	std::vector<wasm::Expression> visit(std::vector<yul::Expression> const& _expressions);
	wasm::Expression visit(yul::Statement const& _statement);
	std::vector<wasm::Expression> visit(std::vector<yul::Statement> const& _statements);

	wasm::FunctionDefinition translateFunction(yul::FunctionDefinition const& _funDef);

	std::string newLabel();

	std::vector<wasm::VariableDeclaration> m_localVariables;
	size_t m_labelCounter = 0;
	std::stack<std::pair<std::string, std::string>> m_breakContinueLabelNames;

	Dialect const& m_dialect;
};

}
