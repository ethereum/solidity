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
#include <libyul/backends/evm/SSAEVMCodeTransform.h>

#include <libyul/backends/evm/ControlFlowGraphBuilder.h>
#include <libyul/backends/evm/SSAControlFlowGraphBuilder.h>
#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/StackLayoutGenerator.h>

#include <libyul/Utilities.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/cxx20.h>

#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take_last.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace {

void debugPrintCFG(SSACFG const& _ssacfg, LivenessData const& _liveness) {
		auto varToString = [&](SSACFG::ValueId _var) {
			if (_var.value == std::numeric_limits<size_t>::max())
				return std::string("INVALID");
			auto const& info = _ssacfg.valueInfo(_var);
			return std::visit(
				util::GenericVisitor{
					[&](SSACFG::UnreachableValue const&) -> std::string {
						return "[unreachable]";
					},
					[&](SSACFG::PhiValue const&) {
						return "p" + std::to_string(_var.value);
					},
					[&](SSACFG::VariableValue const&) {
						return "v" + std::to_string(_var.value);
					},
					[&](SSACFG::LiteralValue const& _literal) {
						std::stringstream str;
						str << _literal.value;
						return str.str();
					}
				},
				info
			);
		};

		auto printGraph = [&](SSACFG::BlockId _root) {
			util::BreadthFirstSearch<SSACFG::BlockId> bfs{{{_root}}};
			bfs.run([&](SSACFG::BlockId _blockId, auto _addChild) {
						//yulAssert(builder.blockInfo(_blockId).sealed, "Some block isn't sealed.");
						auto const& block = _ssacfg.block(_blockId);
						std::cout << "Block b" << _blockId.value << ":" << std::endl;
						std::cout << "[ " << util::joinHumanReadable(_liveness[_blockId].liveIn | ranges::view::transform(varToString)) << " ] => [ "
								  <<  util::joinHumanReadable(_liveness[_blockId].liveOut | ranges::view::transform(varToString)) << " ]" << std::endl;
						for(auto phi: block.phis)
						{
							auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
							yulAssert(phiInfo);
							std::cout << "  " << varToString(phi) << " := phi(" << std::endl;
							yulAssert(_ssacfg.block(phiInfo->block).entries.size() == phiInfo->arguments.size());
							for (auto&& [entry, input]: ranges::zip_view(_ssacfg.block(phiInfo->block).entries, phiInfo->arguments))
								std::cout << "    b" << entry.value << " => " << varToString(input) << std::endl;
							std::cout << "  )" << std::endl;
						}
						for (auto const& op: block.operations)
						{
							std::cout << "  ";
							if (!op.outputs.empty())
								std::cout << util::joinHumanReadable(op.outputs | ranges::view::transform(varToString)) << " := ";

							std::visit(util::GenericVisitor{
/*										   [&](SSACFG::Phi const& _phi) {
											   std::cout << "phi";
											   std::cout << "(" << std::endl;
											   yulAssert(_ssacfg.block(_phi.block).entries.size() == op.inputs.size());
											   for (auto&& [entry, input]: ranges::zip_view(_ssacfg.block(_phi.block).entries, op.inputs))
											   {
												   std::cout << "    b" << entry.value << " => " << varToString(input) << std::endl;
											   }
											   std::cout << "  )" << std::endl;
										   },*/
										   [&](SSACFG::Call const& _call) {
											   std::cout << _call.function.get().name.str();
											   std::cout << "(";
											   std::cout << util::joinHumanReadable(op.inputs | ranges::view::reverse | ranges::view::transform(varToString));
											   std::cout << ")" << std::endl;
										   },
										   [&](SSACFG::BuiltinCall const& _call) {
											   std::cout << _call.builtin.get().name.str();
											   std::cout << "(";
											   std::cout << util::joinHumanReadable(op.inputs | ranges::view::reverse | ranges::view::transform(varToString));
											   std::cout << ")" << std::endl;
										   }
									   }, op.kind);
						}
						std::visit(util::GenericVisitor{
									   [&](SSACFG::BasicBlock::MainExit const&)
									   {
										   std::cout << "  [MAIN EXIT]" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::Jump const& _jump)
									   {
										   std::cout << "  jump(b" << _jump.target.value << ")" << std::endl;
										   _addChild(_jump.target);
									   },
									   [&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
									   {
										   std::cout << "  jumpif(" << varToString(_conditionalJump.condition) << ":" << std::endl;
										   std::cout << "    true => b" << _conditionalJump.nonZero.value << std::endl;
										   std::cout << "    false => b" <<  _conditionalJump.zero.value << std::endl;
										   std::cout << "  )" << std::endl;
										   _addChild(_conditionalJump.nonZero);
										   _addChild(_conditionalJump.zero);
									   },
									   [&](SSACFG::BasicBlock::JumpTable const& _jumpTable)
									   {
										   std::cout << "  jumpv(" << varToString(_jumpTable.value) << ":" << std::endl;
										   for (auto [v, b]: _jumpTable.cases)
										   {
											   std::cout << "    " << v << " => b" << b.value << std::endl;
											   _addChild(b);
										   }
										   _addChild(_jumpTable.defaultCase);
										   std::cout << "    _ => b" << _jumpTable.defaultCase.value << std::endl;
										   std::cout << "  )" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::FunctionReturn const& _return)
									   {
										   std::cout << "  leave(" << util::joinHumanReadable(_return.returnValues | ranges::view::transform(varToString)) << ")" << std::endl;
									   },
									   [&](SSACFG::BasicBlock::Terminated const&) { std::cout << "  [TERMINATED]" << std::endl; }
								   }, block.exit);

						std::cout << std::endl;
					});
		};
		printGraph(SSACFG::BlockId{0});
		for (auto function: _ssacfg.functions)
		{
			auto& info = _ssacfg.functionInfos.at(&function.get());
			std::cout << "FUNCTION " << function.get().name.str() << std::endl;
			printGraph(info.entry);
		}
}

// TODO: note: this is a naive calculation with very bad complexity that
// should be replaced by a proper algorithm exploiting the SSA properties.
// There are linear-time algorithms for this.
LivenessData calculateLiveness(SSACFG& _ssacfg)
{
	LivenessData blockData(_ssacfg.numBlocks());
	auto analyze = [&](SSACFG::BlockId _root) {
		while(true)
		{
			bool allSame = true;

			std::set<SSACFG::BlockId> visited;
			std::list<SSACFG::BlockId> toVisit{{_root}};

			std::vector<SSACFG::BlockId> dfsOrder;
			auto dfs = [&, visited = std::set<SSACFG::BlockId>{}](SSACFG::BlockId _block, auto& _recurse) mutable -> void {
				if (visited.count(_block))
					return;
				visited.emplace(_block);
				dfsOrder.emplace_back(_block);
				_ssacfg.block(_block).forEachExit([&](SSACFG::BlockId _exit) { return _recurse(_exit, _recurse); });
			};
			dfs(_root, dfs);

			for (SSACFG::BlockId block: dfsOrder | ranges::view::reverse)
			{
				auto previousLiveOut = blockData[block].liveOut;
				auto previousLiveIn = blockData[block].liveIn;
				std::visit(util::GenericVisitor{
							   [&](SSACFG::BasicBlock::MainExit const&) {
							   },
							   [&](SSACFG::BasicBlock::Jump const&) {},
							   [&](SSACFG::BasicBlock::ConditionalJump const&) {},
							   [&](SSACFG::BasicBlock::JumpTable const&) {},
							   [&](SSACFG::BasicBlock::FunctionReturn const& _return)
							   {
								   blockData[block].liveOut += _return.returnValues | ranges::to<std::set>;
							   },
							   [&](SSACFG::BasicBlock::Terminated const&)
							   {
							   },
						   }, _ssacfg.block(block).exit);
				_ssacfg.block(block).forEachExit([&](SSACFG::BlockId _exit) {
					std::optional<size_t> entryIndex;
					for (auto&& [i, exitEntries]: _ssacfg.block(_exit).entries | ranges::view::enumerate)
						if (exitEntries == block)
						{
							yulAssert(!entryIndex.has_value());
							entryIndex = i;
						}
					yulAssert(entryIndex.has_value());
					blockData[block].liveOut += blockData[_exit].liveIn;
					for (auto phi: _ssacfg.block(_exit).phis)
					{
						auto const* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
						yulAssert(phiInfo);
						blockData[block].liveOut.insert(phiInfo->arguments.at(*entryIndex));
					}
				});

				std::set<SSACFG::ValueId> invars = blockData[block].liveOut;
				for(auto const& op: _ssacfg.block(block).operations | ranges::view::reverse)
				{
					invars -= op.outputs;
					invars += op.inputs;
				}
				for (auto phi: _ssacfg.block(block).phis)
				{
					auto* phiInfo = std::get_if<SSACFG::PhiValue>(&_ssacfg.valueInfo(phi));
					yulAssert(phiInfo);
					invars.erase(phi);
				}
				// TODO: don't add them in the first place
				for (auto it = invars.begin(); it != invars.end();)
				{
					if (std::holds_alternative<SSACFG::LiteralValue>(_ssacfg.valueInfo(*it)))
						it = invars.erase(it);
					else
						++it;
				}
				// TODO: don't add them in the first place
				{
					auto& liveOut = blockData[block].liveOut;
					for (auto it = liveOut.begin(); it != liveOut.end();)
					{
						if (std::holds_alternative<SSACFG::LiteralValue>(_ssacfg.valueInfo(*it)))
							it = liveOut.erase(it);
						else
							++it;
					}
				}
				blockData[block].liveIn += invars;

				if (blockData[block].liveOut != previousLiveOut || blockData[block].liveIn != previousLiveIn)
					allSame = false;
			}

			// Continue until the sets all saturate. Horrible runtime
			// TODO: replace by proper SSA-based liveness analysis.
			if (allSame)
				break;
		}
	};

	analyze({SSACFG::BlockId{0}});
	for (auto function: _ssacfg.functions)
	{
		auto const& info = _ssacfg.functionInfos.at(&function.get());
		analyze(info.entry);
	}
	return blockData;
}

}

std::vector<StackTooDeepError> SSAEVMCodeTransform::run(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions
)
{
	std::unique_ptr<SSACFG> ssacfg = SSAControlFlowGraphBuilder::build(_analysisInfo, _dialect, _block);
	auto const liveness = calculateLiveness(*ssacfg);
	debugPrintCFG(*ssacfg, liveness);

	SSAEVMCodeTransform optimizedCodeTransform(
		_assembly,
		_builtinContext,
		_useNamedLabelsForFunctions,
		*ssacfg,
		liveness
	);
	optimizedCodeTransform(SSACFG::BlockId{0});
	for (auto function: ssacfg->functions)
		optimizedCodeTransform(ssacfg->functionInfos.at(&function.get()));
	return std::move(optimizedCodeTransform.m_stackErrors);
}

SSAEVMCodeTransform::SSAEVMCodeTransform(
	AbstractAssembly& _assembly,
	BuiltinContext& _builtinContext,
	UseNamedLabels _useNamedLabelsForFunctions,
	SSACFG const& _cfg,
	LivenessData const& _livenessData
):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_cfg(_cfg),
	m_livenessData(_livenessData),
	m_functionLabels([&](){
		std::map<SSACFG::FunctionInfo const*, AbstractAssembly::LabelID> functionLabels;
		std::set<YulString> assignedFunctionNames;
		for (auto function: m_cfg.functions)
		{
			auto const& functionInfo = m_cfg.functionInfos.at(&function.get());
			bool nameAlreadySeen = !assignedFunctionNames.insert(function.get().name).second;
			if (_useNamedLabelsForFunctions == UseNamedLabels::YesAndForceUnique)
				yulAssert(!nameAlreadySeen);
			bool useNamedLabel = _useNamedLabelsForFunctions != UseNamedLabels::Never && !nameAlreadySeen;
			functionLabels[&functionInfo] = useNamedLabel ?
				m_assembly.namedLabel(
					function.get().name.str(),
					function.get().arguments.size(),
					function.get().returns.size(),
					functionInfo.debugData ? functionInfo.debugData->astID : std::nullopt
				) :
				m_assembly.newLabelId();
		}
		return functionLabels;
	}()),
	m_blockData(_cfg.numBlocks())
{
}

AbstractAssembly::LabelID SSAEVMCodeTransform::getFunctionLabel(Scope::Function const& _function)
{
	return m_functionLabels.at(&m_cfg.functionInfos.at(&_function));
}

void SSAEVMCodeTransform::operator()(SSACFG::BlockId _block)
{
	if (blockData(_block).generated)
		return;

	ScopedSaveAndRestore stackSave{m_stack, {}};

	blockData(_block).label = m_assembly.newLabelId();

	blockData(_block).stackIn = liveness(_block).liveIn | ranges::to<std::vector>;


}

void SSAEVMCodeTransform::operator()(SSACFG::FunctionInfo const& _functionInfo)
{
	(void)_functionInfo;
}
