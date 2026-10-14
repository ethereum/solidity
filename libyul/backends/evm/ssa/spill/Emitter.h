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
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/ssa/SSACFGTypes.h>
#include <libyul/backends/evm/ssa/spill/MemoryAddressing.h>

#include <libevmasm/Instruction.h>

namespace solidity::yul::ssa::spill
{

/// Emits the two memory accesses that move a spilled variable between the stack and its reserved memory slot.
class Emitter
{
public:
	Emitter(
		MemoryAddressing const& _addressing,
		FunctionGraphID const _cfgIdx,
		AbstractAssembly& _assembly
	):
		m_addressing(&_addressing),
		m_cfgIdx(_cfgIdx),
		m_assembly(&_assembly)
	{
	}

	[[nodiscard]] bool hasAddress(SpillKey const _key) const
	{
		return m_addressing->hasAddress(m_cfgIdx, _key);
	}

	/// Materializes the spilled variable `_key` on the stack top: `PUSH addr; MLOAD`. Dual of emitStore.
	void emitLoad(SpillKey const _key) const
	{
		m_assembly->appendConstant(m_addressing->addressOf(m_cfgIdx, _key));
		m_assembly->appendInstruction(evmasm::Instruction::MLOAD);
	}

	/// Writes the stack-top value into the spill slot of `_key`, consuming it: `PUSH addr; MSTORE`.
	/// Dual of emitLoad. The value must already be on the stack top.
	void emitStore(SpillKey const _key) const
	{
		m_assembly->appendConstant(m_addressing->addressOf(m_cfgIdx, _key));
		m_assembly->appendInstruction(evmasm::Instruction::MSTORE);
	}

private:
	MemoryAddressing const* m_addressing;
	FunctionGraphID m_cfgIdx;
	AbstractAssembly* m_assembly;
};

}
