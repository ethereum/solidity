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
 * Optimiser component that undoes what the ExpressionSplitter did, i.e.
 * it more or less inlines variable declarations.
 */
#pragma once

#include <libyul/ASTDataForward.h>

#include <libyul/optimiser/ASTWalker.h>

#include <map>

namespace dev
{
namespace yul
{

class NameCollector;


/**
 * Optimiser component that modifies an AST in place, turning sequences
 * of variable declarations into complex expressions, if the variables
 * are declared in the right order. This component does the opposite
 * of ExpressionSplitter.
 * Since the order of opcode or function evaluation is unchanged,
 * this transformation does not need to care about conflicting opcodes.
 *
 * Code of the form
 *
 * let a1 := mload(y)
 * let a2 := mul(x, 4)
 * sstore(a2, a1)
 *
 * is transformed into
 *
 * sstore(mul(x, 4), mload(y))
 *
 * The transformation is not applied to loop conditions, because those are
 * evaluated with each loop.
 *
 * The component can be applied to sub-blocks of the AST, you do not
 * need to pass a full AST.
 *
 * Prerequisites: Disambiguator
 *
 * Implementation note: We visit the AST, modifying it in place.
 * The class starts counting references and will only replace variables
 * that have exactly one reference. It keeps a "latest statement pointer"
 * which always points to the statement right before the current statement.
 * Any function call or opcode will reset this pointer. If an identifier
 * is encountered that was declared in the "latest statement", it is replaced
 * by the value of the declaration, the "latest statement" is replaced
 * by an empty block and the pointer is decremented.
 * A block also resets the latest statement pointer.
 */
class ExpressionJoiner: public ASTModifier
{
public:
	static void run(Block& _ast);

private:
	explicit ExpressionJoiner(Block& _ast);

	void operator()(Block& _block) override;
	void operator()(FunctionalInstruction&) override;
	void operator()(FunctionCall&) override;

	using ASTModifier::visit;
	void visit(Expression& _e) override;

	void handleArguments(std::vector<Expression>& _arguments);

	void decrementLatestStatementPointer();
	void resetLatestStatementPointer();
	Statement* latestStatement();
	bool isLatestStatementVarDeclJoinable(Identifier const& _identifier);

private:
	Block* m_currentBlock = nullptr;            ///< Pointer to currently block holding the visiting statement.
	size_t m_latestStatementInBlock = 0;        ///< Offset to m_currentBlock's statements of the last visited statement.
	std::map<std::string, size_t> m_references; ///< Holds reference counts to all variable declarations in current block.
};

}
}
