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

#include <libyul/Utilities.h>
#include <libyul/YulControlFlowGraphExporter.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/Numeric.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

YulControlFlowGraphExporter::YulControlFlowGraphExporter(ControlFlow const& _controlFlow): m_controlFlow(_controlFlow)
{
}

std::string YulControlFlowGraphExporter::varToString(SSACFG const& _cfg, SSACFG::ValueId _var)
{
	if (_var.value == std::numeric_limits<size_t>::max())
		return std::string("INVALID");
	auto const& info = _cfg.valueInfo(_var);
	return std::visit(
		util::GenericVisitor{
			[&](SSACFG::UnreachableValue const&) -> std::string {
				return "[unreachable]";
			},
			[&](SSACFG::LiteralValue const& _literal) {
				return toCompactHexWithPrefix(_literal.value);
			},
			[&](auto const&) {
				return "v" + std::to_string(_var.value);
			}
		},
		info
	);
}

Json YulControlFlowGraphExporter::run()
{
	Json yulObjectJson = Json::object();
	yulObjectJson["blocks"] = exportBlock(*m_controlFlow.mainGraph, SSACFG::BlockId{0});

	Json functionsJson = Json::object();
	for (auto const& [function, functionGraph]: m_controlFlow.functionGraphMapping)
		functionsJson[function->name.str()] = exportFunction(*functionGraph);
	yulObjectJson["functions"] = functionsJson;

	return yulObjectJson;
}

Json YulControlFlowGraphExporter::exportFunction(SSACFG const& _cfg)
{
	Json functionJson = Json::object();
	functionJson["type"] = "Function";
	functionJson["entry"] = "Block" + std::to_string(_cfg.entry.value);
	functionJson["arguments"] = Json::array();
	for (auto const& [arg, valueId]: _cfg.arguments)
		functionJson["arguments"].emplace_back(arg.get().name.str());
	functionJson["returns"] = Json::array();
	for (auto const& ret: _cfg.returns)
		functionJson["returns"].emplace_back(ret.get().name.str());
	functionJson["blocks"] = exportBlock(_cfg, _cfg.entry);
	return functionJson;
}

Json YulControlFlowGraphExporter::exportBlock(SSACFG const& _cfg, SSACFG::BlockId _entryId)
{
	Json blocksJson = Json::array();
	util::BreadthFirstSearch<SSACFG::BlockId> bfs{{{_entryId}}};
	bfs.run([&](SSACFG::BlockId _blockId, auto _addChild) {
		auto const& block = _cfg.block(_blockId);
		// Convert current block to JSON
		Json blockJson = toJson(_cfg, _blockId);

		Json exitBlockJson = Json::object();
		std::visit(util::GenericVisitor{
			[&](SSACFG::BasicBlock::MainExit const&) {
				exitBlockJson["targets"] = { "Block" + std::to_string(_blockId.value) };
				exitBlockJson["type"] = "MainExit";
			},
			[&](SSACFG::BasicBlock::Jump const& _jump)
			{
				exitBlockJson["targets"] = { "Block" + std::to_string(_jump.target.value) };
				exitBlockJson["type"] = "Jump";
				_addChild(_jump.target);
			},
			[&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
			{
				exitBlockJson["targets"] = { "Block" + std::to_string(_conditionalJump.zero.value), "Block" + std::to_string(_conditionalJump.nonZero.value) };
				exitBlockJson["cond"] = varToString(_cfg, _conditionalJump.condition);
				exitBlockJson["type"] = "ConditionalJump";

				_addChild(_conditionalJump.zero);
				_addChild(_conditionalJump.nonZero);
			},
			[&](SSACFG::BasicBlock::FunctionReturn const& _return) {
				exitBlockJson["instructions"] = toJson(_cfg, _return.returnValues);
				exitBlockJson["targets"] = { "Block" + std::to_string(_blockId.value) };
				exitBlockJson["type"] = "FunctionReturn";
			},
			[&](SSACFG::BasicBlock::Terminated const&) {
				exitBlockJson["targets"] = { "Block" + std::to_string(_blockId.value) };
				exitBlockJson["type"] = "Terminated";
			},
			[&](SSACFG::BasicBlock::JumpTable const&) {
				yulAssert(false);
			}
		}, block.exit);
		blockJson["exit"] = exitBlockJson;
		blocksJson.emplace_back(blockJson);
	});

	return blocksJson;
}

Json YulControlFlowGraphExporter::toJson(SSACFG const& _cfg, SSACFG::BlockId _blockId)
{
	Json blockJson = Json::object();
	auto const& block = _cfg.block(_blockId);

	blockJson["id"] = "Block" + std::to_string(_blockId.value);
	blockJson["instructions"] = Json::array();
	if (!block.phis.empty())
	{
		blockJson["entries"] = block.entries
			| ranges::views::transform([](auto const& entry) { return "Block" + std::to_string(entry.value); })
			| ranges::to<Json::array_t>();
		for (auto const& phi: block.phis)
		{
			auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(phi));
			yulAssert(phiInfo);
			Json phiJson = Json::object();
			phiJson["op"] = "PhiFunction";
			phiJson["in"] = toJson(_cfg, phiInfo->arguments);
			phiJson["out"] = toJson(_cfg, std::vector<SSACFG::ValueId>{phi});
			blockJson["instructions"].push_back(phiJson);
		}
	}
	for (auto const& operation: block.operations)
		blockJson["instructions"].push_back(toJson(blockJson, _cfg, operation));

	return blockJson;
}

Json YulControlFlowGraphExporter::toJson(Json& _ret, SSACFG const& _cfg, SSACFG::Operation const& _operation)
{
	Json opJson = Json::object();
	std::visit(util::GenericVisitor{
		[&](SSACFG::Call const& _call) {
			_ret["type"] = "FunctionCall";
			opJson["op"] = _call.function.get().name.str();
		},
		[&](SSACFG::BuiltinCall const& _call) {
			_ret["type"] = "BuiltinCall";
			Json builtinArgsJson = Json::array();
			auto const& builtin = _call.builtin.get();
			if (!builtin.literalArguments.empty())
			{
				auto const& functionCallArgs = _call.call.get().arguments;
				for (size_t i = 0; i < builtin.literalArguments.size(); ++i)
				{
					std::optional<LiteralKind> const& argument = builtin.literalArguments[i];
					if (argument.has_value() && i < functionCallArgs.size())
					{
						// The function call argument at index i must be a literal if builtin.literalArguments[i] is not nullopt
						yulAssert(std::holds_alternative<Literal>(functionCallArgs[i]));
						builtinArgsJson.push_back(formatLiteral(std::get<Literal>(functionCallArgs[i])));
					}
				}
			}

			if (!builtinArgsJson.empty())
				opJson["builtinArgs"] = builtinArgsJson;

			opJson["op"] = _call.builtin.get().name.str();
		},
	}, _operation.kind);

	opJson["in"] = toJson(_cfg, _operation.inputs);
	opJson["out"] = toJson(_cfg, _operation.outputs);

	return opJson;
}

Json YulControlFlowGraphExporter::toJson(SSACFG const& _cfg, std::vector<SSACFG::ValueId> const& _values)
{
	Json ret = Json::array();
	for (auto const& value: _values)
		ret.push_back(varToString(_cfg, value));
	return ret;
}
