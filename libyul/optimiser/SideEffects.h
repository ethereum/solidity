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

#include <libyul/Dialect.h>

#include <libevmasm/Instruction.h>

#include <set>
#include <map>

namespace yul
{
struct Dialect;

/**
 * Specific class that represents side-effect of code.
 * It is a bounded lattice.
 */
class SideEffects
{
public:
	static SideEffects join(SideEffects, SideEffects);
	static SideEffects top();
	static SideEffects bottom();

	explicit SideEffects(BuiltinFunction const& _builtin);
	explicit SideEffects(dev::eth::Instruction const& _instr);
	SideEffects(SideEffects const&) = default;

	bool movable() const { return m_movable; }
	bool sideEffectFree(bool _allowMSizeModification = false) const
	{
		if (_allowMSizeModification)
			return sideEffectFreeIfNoMSize();
		else
			return m_sideEffectFree;
	}
	bool sideEffectFreeIfNoMSize() const { return m_sideEffectFreeIfNoMSize; }
	bool containsMSize() const { return m_containsMSize; }
	bool invalidatesStorage() const { return m_invalidatesStorage; }
	bool invalidatesMemory() const { return m_invalidatesMemory; }

private:
	SideEffects(
		bool _movable,
		bool _sideEffectFree,
		bool _sideEffectFreeIfNoMSize,
		bool _containsMSize,
		bool _invalidatesStorage,
		bool _invalidatesMemory
	):
		m_movable(_movable),
		m_sideEffectFree(_sideEffectFree),
		m_sideEffectFreeIfNoMSize(_sideEffectFreeIfNoMSize),
		m_containsMSize(_containsMSize),
		m_invalidatesStorage(_invalidatesStorage),
		m_invalidatesMemory(_invalidatesMemory)
	{}

	bool m_movable;
	/// Is the current expression side-effect free, i.e. can be removed
	/// without changing the semantics.
	bool m_sideEffectFree;
	/// Is the current expression side-effect free up to msize, i.e. can be removed
	/// without changing the semantics except for the value returned by the msize instruction.
	bool m_sideEffectFreeIfNoMSize;
	/// Does the current code contain the MSize operation?
	bool m_containsMSize;
	/// If false, storage is guaranteed to be unchanged by the code under all
	/// circumstances.
	bool m_invalidatesStorage;
	bool m_invalidatesMemory;
};

/**
 * Specific AST walker that determines side-effect free-ness and movability of code.
 */
class SideEffectsCollector: public ASTWalker
{
public:
	/// Constructor that will figure out all user defined functions' side effects.
	/// Triggers assertion failure if function definition is missing but the function is called.
	/// Information not updated if AST modified afterwards
	explicit SideEffectsCollector(Dialect const& _dialect, Block const& _ast);

	/// Construct a side effect collector that throws (!?) when user defined functions are called.
	/// Currently acheived by not filling m_functionSideEffects at all. (TODO: should have a more correct way)
	explicit SideEffectsCollector(Dialect const& _dialect);

	using ASTWalker::operator();
	void operator()(FunctionalInstruction const& _functionalInstruction) override;
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(FunctionDefinition const& _functionDefinition) override;

	SideEffects sideEffectsOf(Block const& _block);
	SideEffects sideEffectsOf(Expression const& _expression);
	SideEffects sideEffectsOf(Statement const& _statement);

private:
	Dialect const& m_dialect;
	SideEffects m_sideEffects = SideEffects::bottom();
	std::map<YulString, SideEffects> m_functionSideEffects;
};

}
