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
#include <range/v3/view/map.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{
	std::string variableSlotToString(VariableSlot const& _slot)
	{
		return _slot.variable.get().name.str();
	}

	void validateStackSlot(StackSlot const& _slot)
	{
		std::visit(util::GenericVisitor{
			[](FunctionCallReturnLabelSlot const&) { solAssert(false); },
			[](FunctionReturnLabelSlot const&) { solAssert(false); },
			[](VariableSlot const&) {},
			[](LiteralSlot const&) {},
			[](TemporarySlot const&) { solAssert(false); },
			[](JunkSlot const&) {},
		}, _slot);
	}
}

Json YulControlFlowGraphExporter::operator()(CFG const& _cfg)
{
	Json yulObjectJson = Json::object();
	yulObjectJson["blocks"] = exportBlock(*_cfg.entry);

	Json functionsJson = Json::object();
	for (auto const& functionInfo: _cfg.functionInfo | ranges::views::values)
		functionsJson[functionInfo.function.name.str()] = exportFunction(functionInfo);
	yulObjectJson["functions"] = functionsJson;

	return yulObjectJson;
}

Json YulControlFlowGraphExporter::exportFunction(CFG::FunctionInfo const& _functionInfo)
{
	Json functionJson = Json::object();
	functionJson["type"] = "Function";
	functionJson["entry"] = "Block" + std::to_string(getBlockId(*_functionInfo.entry));
	functionJson["arguments"] = Json::array();
	for (auto const& parameter: _functionInfo.parameters)
		functionJson["arguments"].emplace_back(variableSlotToString(parameter));
	functionJson["returns"] = Json::array();
	for (auto const& returnValue: _functionInfo.returnVariables)
		functionJson["returns"].emplace_back(variableSlotToString(returnValue));
	functionJson["blocks"] = exportBlock(*_functionInfo.entry);
	return functionJson;
}

Json YulControlFlowGraphExporter::exportBlock(CFG::BasicBlock const& _block)
{
	Json blocksJson = Json::array();
	util::BreadthFirstSearch<CFG::BasicBlock const*> bfs{{&_block}};
	bfs.run([&](CFG::BasicBlock const* _block, auto _addChild) {
		// Convert current block to JSON
		blocksJson.emplace_back(toJson(*_block));

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
				//TODO: handle backwards jump?
				_addChild(_jump.target);
			},
			[&](CFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				exitBlockJson["exit"] = { "Block" + std::to_string(getBlockId(*_conditionalJump.zero)), "Block" + std::to_string(getBlockId(*_conditionalJump.nonZero)) };
				exitBlockJson["cond"] = stackSlotToJson(_conditionalJump.condition);
				exitBlockJson["type"] = "ConditionalJump";

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
		blocksJson.emplace_back(exitBlockJson);
	});

	return blocksJson;
}

Json YulControlFlowGraphExporter::toJson(CFG::BasicBlock const& _block)
{
	Json blockJson;
	blockJson["id"] = "Block" + std::to_string(getBlockId(_block));
	blockJson["instructions"] = Json::array();
	for (auto const& operation: _block.operations)
		blockJson["instructions"].push_back(toJson(blockJson, operation));
	blockJson["exit"] = "Block" + std::to_string(getBlockId(_block)) + "Exit";

	return blockJson;
}

Json YulControlFlowGraphExporter::toJson(Json& _ret, CFG::Operation const& _operation)
{
	Json opJson;

	Stack input = _operation.input;
	std::visit(util::GenericVisitor{
		[&](CFG::FunctionCall const& _call) {
			solAssert(!input.empty(), "FunctionCall must have a return label as first input");
			StackSlot const& slot = input.front();
			if (
				std::holds_alternative<FunctionCallReturnLabelSlot>(slot) ||
				std::holds_alternative<FunctionReturnLabelSlot>(slot)
			)
			{
				if (auto* returnLabelSlot = std::get_if<FunctionCallReturnLabelSlot>(&slot))
					solAssert(returnLabelSlot->call.get().functionName.name == _call.function.get().name, "FunctionCallReturnLabelSlot must refer to the same function as the FunctionCall");
				// remove the return label from the input
				input.erase(input.begin());
			}
			_ret["type"] = "FunctionCall";
			opJson["op"] = _call.function.get().name.str();
		},
		[&](CFG::BuiltinCall const& _call) {
			_ret["type"] = "BuiltinCall";
			Json builtinArgsJson = Json::array();
			auto const& builtin = _call.builtin.get();
			if (!builtin.literalArguments.empty())
			{
				auto const& functionCallArgs = _call.functionCall.get().arguments;
				for (size_t i = 0; i < builtin.literalArguments.size(); ++i)
				{
					std::optional<LiteralKind> const& argument = builtin.literalArguments[i];
					if (argument.has_value() && i < functionCallArgs.size())
					{
						// The function call argument at index i must be a literal if builtin.literalArguments[i] is not nullopt
						yulAssert(std::holds_alternative<Literal>(functionCallArgs[i]));
						builtinArgsJson.push_back(std::get<Literal>(functionCallArgs[i]).value.str());
					}
				}
			}

			if (!builtinArgsJson.empty())
				opJson["builtinArgs"] = builtinArgsJson;

			opJson["op"] = _call.functionCall.get().functionName.name.str();
		},
		[&](CFG::Assignment const& _assignment) {
			_ret["type"] = "Assignment";
			Json::array_t assignmentJson;
			for (auto const& variable: _assignment.variables)
				assignmentJson.push_back(stackSlotToString(variable));
			opJson["assignment"] = assignmentJson;
		}
	}, _operation.operation);

	opJson["in"] = Json::array();
	opJson["in"] = stackToJson(input);

	opJson["out"] = Json::array();
	opJson["out"] = stackToJson(_operation.output);

	return opJson;
}

Json YulControlFlowGraphExporter::stackToJson(Stack const& _stack)
{
	Json ret = Json::array();
	for (auto const& slot: _stack)
	{
		validateStackSlot(slot);
		ret.push_back(stackSlotToString(slot));
	}
	return ret;
}

Json YulControlFlowGraphExporter::stackSlotToJson(StackSlot const& _slot)
{
	Json ret = Json::array();
	validateStackSlot(_slot);
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
