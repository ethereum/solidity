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
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into JULIA.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/Exceptions.h>
#include <string>

namespace dev
{
namespace solidity
{

class SourceUnit;

class IRGenerate: private ASTConstVisitor
{
public:
	/// resets the state
	void reset() { m_processed = false; m_body.statements.clear(); }

	/// process a source unit
	void process(SourceUnit const& _source) { solAssert(!m_processed, ""); _source.accept(*this); m_processed = true; }

	/// @returns the JULIA block
	assembly::Block body() const { solAssert(m_processed, ""); return m_body; }

private:
	virtual bool visitNode(ASTNode const&) override
	{
		solUnimplementedAssert(false, "This AST element is not supported yet.");
	}

	virtual bool visit(SourceUnit const&) override { return true; }
	virtual bool visit(PragmaDirective const&) override { return true; }
	virtual bool visit(ContractDefinition const&) override;
	virtual bool visit(FunctionDefinition const&) override;
	virtual void endVisit(FunctionDefinition const&) override;
	virtual bool visit(Block const&) override;
	virtual bool visit(Throw const&) override;
	virtual bool visit(InlineAssembly const&) override;

	void buildDispatcher(ContractDefinition const&);
	void appendFunction(std::string const&);
	assembly::FunctionCall createFunctionCall(std::string const&);
	assembly::Block wrapInBlock(assembly::Statement const&);

	bool m_processed = false;
	assembly::Block m_body;
	assembly::FunctionDefinition m_currentFunction;
};

}
}
