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
	void reset() { m_contracts.clear(); }

	/// process a source unit
	void process(SourceUnit const& _source) { _source.accept(*this); }

	/// @returns the JULIA block
	assembly::Block contract(std::string const& _contractName) const { solAssert(m_contracts.count(_contractName), ""); return *m_contracts.at(_contractName); }

	/// @returns the contract names
	std::vector<std::string> contractNames() const {
		std::vector<std::string> names;
		for (auto const& contract: m_contracts)
			names.push_back(contract.first);
		return names;
	}

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
	assembly::FunctionCall createRevert();

	std::map<std::string, std::shared_ptr<assembly::Block>> m_contracts;
	std::shared_ptr<assembly::Block> m_body;
	assembly::FunctionDefinition m_currentFunction;
};

}
}
