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
 * Module providing metrics for the optimizer.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <liblangutil/EVMVersion.h>

namespace yul
{

struct Dialect;
struct EVMDialect;

/**
 * Metric for the size of code.
 * More specifically, the number of AST nodes.
 * Ignores function definitions while traversing the AST by default.
 * If you want to know the size of a function, you have to invoke this on its body.
 *
 * As an exception, the following AST elements have a cost of zero:
 *  - expression statement (only the expression inside has a cost)
 *  - block (only the statements inside have a cost)
 *  - variable references
 *  - variable declarations (only the right hand side has a cost)
 *  - assignments (only the value has a cost)
 *
 * As another exception, each statement incurs and additional cost of one
 * per jump/branch. This means if, break and continue statements have a cost of 2,
 * switch statements have a cost of 1 plus the number of cases times two,
 * and for loops cost 3.
 */
class CodeSize: public ASTWalker
{
public:
	static size_t codeSize(Statement const& _statement);
	static size_t codeSize(Expression const& _expression);
	static size_t codeSize(Block const& _block);
	static size_t codeSizeIncludingFunctions(Block const& _block);

private:
	CodeSize(bool _ignoreFunctions = true): m_ignoreFunctions(_ignoreFunctions) {}

	void visit(Statement const& _statement) override;
	void visit(Expression const& _expression) override;

private:
	bool m_ignoreFunctions;
	size_t m_size = 0;
};

/**
 * Very rough cost that takes the size and execution cost of code into account.
 * The cost per AST element is one, except for literals where it is the byte size.
 * Function calls cost 50. Instructions cost 0 for 3 or less gas (same as DUP),
 * 2 for up to 10 and 50 otherwise.
 */
class CodeCost: public ASTWalker
{
public:
	static size_t codeCost(Dialect const& _dialect, Expression const& _expression);

private:
	CodeCost(Dialect const& _dialect): m_dialect(_dialect) {}

	void operator()(FunctionCall const& _funCall) override;
	void operator()(Literal const& _literal) override;
	void visit(Statement const& _statement) override;
	void visit(Expression const& _expression) override;

private:
	void addInstructionCost(dev::eth::Instruction _instruction);

	Dialect const& m_dialect;
	size_t m_cost = 0;
};

/**
 * Counts the number of assignments to every variable.
 * Only works after running the Disambiguator.
 */
class AssignmentCounter: public ASTWalker
{
public:
	using ASTWalker::operator();
	void operator()(Assignment const& _assignment) override;
	std::size_t assignmentCount(YulString _name) const;
private:
	std::map<YulString, size_t> m_assignmentCounters;
};

}
