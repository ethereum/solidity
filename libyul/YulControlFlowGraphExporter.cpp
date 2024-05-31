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

#include <libyul/YulControlFlowGraphExporter.h>
#include <libyul/backends/evm/StackHelpers.h>

#include <libsolutil/Algorithms.h>

#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

void YulControlFlowGraphExporter::operator()(CFG::BasicBlock const& _entry)
{
	Json ret = Json::array();
	util::BreadthFirstSearch<CFG::BasicBlock const*>{{&_entry}}.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		// Convert current block to JSON
		ret.push_back(toJson(*_block));

		Json exitBlockJson;
		exitBlockJson["id"] = "Block" + std::to_string(getBlockId(*_block)) + "Exit";
		exitBlockJson["instructions"] = Json::array();
		std::visit(util::GenericVisitor{
			[&](CFG::BasicBlock::MainExit const&) {
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_block)) };
				exitBlockJson["type"] = "MainExit";
			},
			[&](CFG::BasicBlock::Jump const& _jump)
			{
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_jump.target)) };
				exitBlockJson["type"] = "Jump";
				ret.push_back(exitBlockJson);

				//TODO: handle backwards jump?
				_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_conditionalJump.zero)), "Block" + std::to_string(getBlockId(*_conditionalJump.nonZero)) };
				exitBlockJson["cond"] = stackSlotToJson(_conditionalJump.condition);
				exitBlockJson["type"] = "ConditionalJump";
				ret.push_back(exitBlockJson);

				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](CFG::BasicBlock::FunctionReturn const& _return) {
				exitBlockJson["instructions"].push_back(_return.info->function.name.str());
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_block)) };
				exitBlockJson["type"] = "FunctionReturn";
			},
			[&](CFG::BasicBlock::Terminated const&) {
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_block)) };
				exitBlockJson["type"] = "Terminated";
			},
		}, _block->exit);
		ret.push_back(exitBlockJson);
	});

	// TODO: return Json, remove debug output, and add tests
	std::cout << ret << std::endl;
}

Json YulControlFlowGraphExporter::toJson(CFG::BasicBlock const& _block)
{
	Json blockJson;
	blockJson["id"] = "Block" + std::to_string(getBlockId(_block));
	blockJson["instructions"] = Json::array();
	for (auto const& operation: _block.operations)
		blockJson["instructions"].push_back(toJson(operation));
	blockJson["exit"] = "Block" + std::to_string(getBlockId(_block)) + "Exit";
	blockJson["type"] = "BasicBlock";

	return blockJson;
}

Json YulControlFlowGraphExporter::toJson(CFG::Operation const& _operation)
{
	Json opJson;
	std::visit(util::GenericVisitor{
		[&](CFG::FunctionCall const& _call) {
			opJson["op"] = _call.function.get().name.str();
		},
		[&](CFG::BuiltinCall const& _call) {
			opJson["op"] = _call.functionCall.get().functionName.name.str();
		},
		[&](CFG::Assignment const& _assignment) {
			Json::array_t assignmentJson;
			for (auto const& variable: _assignment.variables)
				assignmentJson.push_back(stackSlotToString(variable));
			opJson["assignment"] = assignmentJson;
		}
	}, _operation.operation);

	opJson["in"] = Json::array();
	opJson["in"] = stackToJson(_operation.input);

	opJson["out"] = Json::array();
	opJson["out"] = stackToJson(_operation.output);

	return opJson;
}

Json YulControlFlowGraphExporter::stackToJson(Stack const& _stack)
{
	Json ret = Json::array();
	for (auto const& slot: _stack)
		ret.push_back(stackSlotToString(slot));
	return ret;
}

Json YulControlFlowGraphExporter::stackSlotToJson(StackSlot const& _slot)
{
	Json ret = Json::array();
	ret.push_back(stackSlotToString(_slot));
	return ret;
}

size_t YulControlFlowGraphExporter::getBlockId(CFG::BasicBlock const& _block)
{
	if (size_t* id = util::valueOrNullptr(m_blockIds, &_block))
		return *id;
	size_t id = m_blockIds[&_block] = m_blockCount++;
	return id;
}
