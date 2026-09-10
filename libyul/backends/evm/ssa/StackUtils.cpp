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

#include <libyul/backends/evm/ssa/StackUtils.h>

#include <libevmasm/GasMeter.h>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/zip.hpp>

#include <fmt/ranges.h>

using namespace solidity::yul::ssa;

std::size_t solidity::yul::ssa::stackOpsGas(SSACFG const& _cfg, ShuffleTrace const& _trace)
{
	auto const evmVersion = _cfg.evmDialect.evmVersion();
	auto const runGas = [&](evmasm::Instruction const _instruction) {
		return evmasm::GasMeter::runGas(_instruction, evmVersion);
	};
	std::size_t gas = 0;
	for (ShuffleOp const& op: _trace)
		switch (op.kind)
		{
		case ShuffleOp::Kind::Swap:
			gas += evmasm::GasMeter::swapGas(op.depth, evmVersion);
			break;
		case ShuffleOp::Kind::Dup:
			gas += evmasm::GasMeter::dupGas(op.depth, evmVersion);
			break;
		case ShuffleOp::Kind::Pop:
			gas += runGas(evmasm::Instruction::POP);
			break;
		case ShuffleOp::Kind::Push:
			if (op.slot.isLiteralValue())
				gas += runGas(evmasm::pushInstruction(numberEncodingSize(_cfg.literalPayload(op.slot.value()))));
			else if (op.slot.isJunk())
				gas += runGas(evmVersion.hasPush0() ? evmasm::Instruction::PUSH0 : evmasm::Instruction::CODESIZE);
			else
			{
				yulAssert(op.slot.isFunctionCallReturnLabel(), "unexpected slot kind in shuffle trace push");
				// this is a jump dest, we don't really know yet how big it is going to be, just assume that it fits
				// into a 2-byte number
				gas += runGas(evmasm::Instruction::PUSH2);
			}
			break;
		case ShuffleOp::Kind::Load:
			gas += runGas(evmasm::Instruction::PUSH32) + runGas(evmasm::Instruction::MLOAD);
			break;
		case ShuffleOp::Kind::Store:
			gas += runGas(evmasm::Instruction::PUSH32) + runGas(evmasm::Instruction::MSTORE);
			break;
		}
	return gas;
}

StackData solidity::yul::ssa::stackPreImage(SSACFG const& _cfg, StackData _stack, PhiInverse const& _phiInverse)
{
	if (!_phiInverse.noOp())
		for (auto& slot: _stack)
			if (slot.isValue())
			{
				auto const preImage = _phiInverse(slot.value());
				slot = StackSlot::makeValue(_cfg, preImage);
			}
	return _stack;
}

CallSites solidity::yul::ssa::gatherCallSites(SSACFG const& _cfg)
{
	CallSites result;
	std::vector<std::uint8_t> visited(_cfg.numBlocks(), false);
	visited[_cfg.entry.value] = true;
	std::vector<SSACFG::BlockId> toVisit;
	toVisit.reserve(_cfg.numBlocks());
	toVisit.push_back(_cfg.entry);

	while (!toVisit.empty())
	{
		auto const blockId = toVisit.back();
		toVisit.pop_back();
		auto const& block = _cfg.block(blockId);
		block.forEachExit([&toVisit, &visited](SSACFG::BlockId const& _exitBlockId){
			if (!visited[_exitBlockId.value])
			{
				visited[_exitBlockId.value] = true;
				toVisit.push_back(_exitBlockId);
			}
		});

		for (InstId const instId: block.instructions)
		{
			auto const& inst = _cfg.inst(instId);
			if (inst.opcode != InstOpcode::Call)
				continue;
			if (_cfg.callPayload(instId).canContinue)
				result.addCallSite(instId);
		}
	}
	return result;
}

std::string ValidationResult::formatErrors() const
{
	return fmt::format("{}", fmt::join(m_errors, "\n"));
}

ValidationResult solidity::yul::ssa::checkLayoutCompatibility(StackData const& _current, StackData const& _desired)
{
	ValidationResult result;
	if (_current.size() != _desired.size())
		return result.addError(fmt::format(
			"size mismatch: {} = len({}) =/= len({}) = {}",
			_current.size(), stackToString(_current), stackToString(_desired), _desired.size()
		));
	for (auto&& [index, currentSlot, desiredSlot]: ranges::zip_view(ranges::views::iota(0), _current, _desired))
		if (!desiredSlot.isJunk() && currentSlot != desiredSlot)
			result.addError(fmt::format(
				"stack element mismatch: {} = {}[{}] =/= {}[{}] = {}",
				slotToString(currentSlot),
				stackToString(_current),
				index,
				stackToString(_desired),
				index,
				slotToString(desiredSlot)
			));
	return result;
}
