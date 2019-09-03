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
 * Specific AST walkers that collect semantical facts.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/SideEffects.h>
#include <libyul/AsmData.h>

#include <set>

namespace yul
{
struct Dialect;

/**
 * Specific AST walker that determines side-effect free-ness and movability of code.
 * Enters into function definitions.
 */
class SideEffectsCollector: public ASTWalker
{
public:
	explicit SideEffectsCollector(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> const* _functionSideEffects = nullptr
	): m_dialect(_dialect), m_functionSideEffects(_functionSideEffects) {}
	SideEffectsCollector(
		Dialect const& _dialect,
		Expression const& _expression,
		std::map<YulString, SideEffects> const* _functionSideEffects = nullptr
	);
	SideEffectsCollector(Dialect const& _dialect, Statement const& _statement);
	SideEffectsCollector(Dialect const& _dialect, Block const& _ast);

	using ASTWalker::operator();
	void operator()(FunctionalInstruction const& _functionalInstruction) override;
	void operator()(FunctionCall const& _functionCall) override;

	bool movable() const { return m_sideEffects.movable; }
	bool sideEffectFree(bool _allowMSizeModification = false) const
	{
		if (_allowMSizeModification)
			return sideEffectFreeIfNoMSize();
		else
			return m_sideEffects.sideEffectFree;
	}
	bool sideEffectFreeIfNoMSize() const { return m_sideEffects.sideEffectFreeIfNoMSize; }
	bool invalidatesStorage() const { return m_sideEffects.invalidatesStorage; }
	bool invalidatesMemory() const { return m_sideEffects.invalidatesMemory; }

private:
	Dialect const& m_dialect;
	std::map<YulString, SideEffects> const* m_functionSideEffects = nullptr;
	SideEffects m_sideEffects;
};

/**
 * This class can be used to determine the side-effects of user-defined functions.
 *
 * It is given a dialect and a mapping that represents the direct calls from user-defined
 * functions to other user-defined functions and built-in functions.
 */
class SideEffectsPropagator
{
public:
	static std::map<YulString, SideEffects> sideEffects(
		Dialect const& _dialect,
		std::map<YulString, std::set<YulString>> const& _directCallGraph
	);
};

/**
 * Class that can be used to find out if certain code contains the MSize instruction.
 *
 * Note that this is a purely syntactic property meaning that even if this is false,
 * the code can still contain calls to functions that contain the msize instruction.
 *
 * The only safe way to determine this is by passing the full AST.
 */
class MSizeFinder: public ASTWalker
{
public:
	static bool containsMSize(Dialect const& _dialect, Block const& _ast);

	using ASTWalker::operator();
	void operator()(FunctionalInstruction const& _instr);
	void operator()(FunctionCall const& _funCall);

private:
	MSizeFinder(Dialect const& _dialect): m_dialect(_dialect) {}
	Dialect const& m_dialect;
	bool m_msizeFound = false;
};

/**
 * Specific AST walker that determines whether an expression is movable
 * and collects the referenced variables.
 * Can only be used on expressions.
 */
class MovableChecker: public SideEffectsCollector
{
public:
	explicit MovableChecker(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> const* _functionSideEffects = nullptr
	): SideEffectsCollector(_dialect, _functionSideEffects) {}
	MovableChecker(Dialect const& _dialect, Expression const& _expression);

	void operator()(Identifier const& _identifier) override;

	/// Disallow visiting anything apart from Expressions (this throws).
	void visit(Statement const&) override;
	using ASTWalker::visit;

	std::set<YulString> const& referencedVariables() const { return m_variableReferences; }

private:
	/// Which variables the current expression references.
	std::set<YulString> m_variableReferences;
};


/**
 * Helper class to find "irregular" control flow.
 * This includes termination, break and continue.
 */
class TerminationFinder
{
public:
	enum class ControlFlow { FlowOut, Break, Continue, Terminate };

	TerminationFinder(Dialect const& _dialect): m_dialect(_dialect) {}

	/// @returns the index of the first statement in the provided sequence
	/// that is an unconditional ``break``, ``continue`` or a
	/// call to a terminating builtin function.
	/// If control flow can continue at the end of the list,
	/// returns `FlowOut` and ``size_t(-1)``.
	/// The function might return ``FlowOut`` even though control
	/// flow cannot actually continue.
	std::pair<ControlFlow, size_t> firstUnconditionalControlFlowChange(
		std::vector<Statement> const& _statements
	);

	/// @returns the control flow type of the given statement.
	/// This function could return FlowOut even if control flow never continues.
	ControlFlow controlFlowKind(Statement const& _statement);

	/// @returns true if the expression statement is a direct
	/// call to a builtin terminating function like
	/// ``stop``, ``revert`` or ``return``.
	bool isTerminatingBuiltin(ExpressionStatement const& _exprStmnt);

private:
	Dialect const& m_dialect;
};

}
