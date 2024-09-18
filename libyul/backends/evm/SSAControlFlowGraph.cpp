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

#include <libyul/backends/evm/SSAControlFlowGraph.h>

#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-compare"
#include <fmt/ranges.h>
#pragma GCC diagnostic pop

#include <range/v3/view/zip.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{
class SSACFGPrinter
{
public:
	SSACFGPrinter(SSACFG const& _cfg, SSACFG::BlockId _blockId)
	{
		printBlock(_cfg, _blockId, 0);
	}
	SSACFGPrinter(SSACFG const& _cfg, size_t _functionIndex, Scope::Function const& _function)
	{
		printFunction(_cfg, _functionIndex, _function);
	}
	friend std::ostream& operator<<(std::ostream& stream, SSACFGPrinter const& printer) {
		stream << printer.m_result.str();
		return stream;
	}
private:
	static std::string varToString(SSACFG const& _cfg, SSACFG::ValueId _var) {
		if (_var.value == std::numeric_limits<size_t>::max())
			return "INVALID";
		auto const& info = _cfg.valueInfo(_var);
		return std::visit(
			GenericVisitor{
				[&](SSACFG::UnreachableValue const&) -> std::string {
					return "[unreachable]";
				},
				[&](SSACFG::PhiValue const&) -> std::string {
					return fmt::format("v{}", _var.value);
				},
				[&](SSACFG::VariableValue const&) -> std::string {
					return fmt::format("v{}", _var.value);
				},
				[&](SSACFG::LiteralValue const& _literal) -> std::string {
					return formatNumberReadable(_literal.value);
				}
			},
			info
		);
	}

	static std::string formatPhi(SSACFG const& _cfg, SSACFG::PhiValue const& _phiValue)
	{
		auto const transform = [&](SSACFG::ValueId const& valueId) { return varToString(_cfg, valueId); };
		std::vector<std::string> formattedArgs;
		formattedArgs.reserve(_phiValue.arguments.size());
		for (auto const& [arg, entry]: ranges::zip_view(_phiValue.arguments | ranges::views::transform(transform), _cfg.block(_phiValue.block).entries))
			formattedArgs.push_back(fmt::format("Block {} => {}", entry.value, arg));
		if (!formattedArgs.empty())
			return fmt::format("φ(\\l\\\n\t{}\\l\\\n)", fmt::join(formattedArgs, ",\\l\\\n\t"));
		else
			return "φ()";
	}

	void writeBlock(SSACFG const& _cfg, SSACFG::BlockId const& _id, SSACFG::BasicBlock const& _block, size_t const _functionIndex)
	{
		auto const valueToString = [&](SSACFG::ValueId const& valueId) { return varToString(_cfg, valueId); };
		bool entryBlock = _id.value == 0 && _functionIndex == 0;
		if (entryBlock)
		{
			m_result << fmt::format("Entry{} [label=\"Entry\"];\n", _functionIndex);
			m_result << fmt::format("Entry{} -> Block{}_{};\n", _functionIndex, _functionIndex, _id.value);
		}
		{
			m_result << fmt::format("Block{1}_{0} [label=\"\\\nBlock {0}\\n", _id.value, _functionIndex);
			for (auto const& phi: _block.phis)
			{
				auto const* phiValue = std::get_if<SSACFG::PhiValue>(&_cfg.valueInfo(phi));
				solAssert(phiValue);
				m_result << fmt::format("v{} := {}\\l\\\n", phi.value, formatPhi(_cfg, *phiValue));
			}
			for (auto const& operation: _block.operations)
			{
				std::string const label = std::visit(GenericVisitor{
					[&](SSACFG::Call const& _call) {
						return _call.function.get().name.str();
					},
					[&](SSACFG::BuiltinCall const& _call) {
						return _call.builtin.get().name.str();
					},
				}, operation.kind);
				if (!operation.outputs.empty())
					m_result << fmt::format(
						"{} := ",
						fmt::join(operation.outputs | ranges::views::transform(valueToString), ", ")
					);
				m_result << fmt::format(
					"{}({})\\l\\\n",
					label,
					fmt::join(operation.inputs | ranges::views::transform(valueToString), ", ")
				);
			}
			m_result << "\"];\n";
			std::visit(GenericVisitor{
				[&](SSACFG::BasicBlock::MainExit const&)
				{
					m_result << fmt::format("Block{}_{}Exit [label=\"MainExit\"];\n", _functionIndex, _id.value);
					m_result << fmt::format("Block{1}_{0} -> Block{1}_{0}Exit;\n", _id.value, _functionIndex);
				},
				[&](SSACFG::BasicBlock::Jump const& _jump)
				{
					m_result << fmt::format("Block{1}_{0} -> Block{1}_{0}Exit [arrowhead=none];\n", _id.value, _functionIndex);
					m_result << fmt::format("Block{}_{}Exit [label=\"Jump\" shape=oval];\n", _functionIndex, _id.value);
					m_result << fmt::format("Block{}_{}Exit -> Block{}_{};\n", _functionIndex, _id.value, _functionIndex, _jump.target.value);
				},
				[&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
				{
					m_result << "Block" << _functionIndex << "_" << _id.value << " -> Block" << _functionIndex << "_" << _id.value << "Exit;\n";
					m_result << "Block" << _functionIndex << "_" << _id.value << "Exit [label=\"{ If ";
					m_result << varToString(_cfg, _conditionalJump.condition);
					m_result << "| { <0> Zero | <1> NonZero }}\" shape=Mrecord];\n";
					m_result << "Block" << _functionIndex << "_" << _id.value;
					m_result << "Exit:0 -> Block" << _functionIndex << "_" << _conditionalJump.zero.value << ";\n";
					m_result << "Block" << _functionIndex << "_" << _id.value;
					m_result << "Exit:1 -> Block" << _functionIndex << "_" << _conditionalJump.nonZero.value << ";\n";
				},
				[&](SSACFG::BasicBlock::JumpTable const& jt)
				{
					m_result << "Block" << _functionIndex << "_" << _id.value << " -> Block" << _functionIndex << "_" << _id.value << "Exit;\n";
					std::string options;
					for (auto const& jumpCase: jt.cases)
					{
						if (!options.empty())
							options += " | ";
						options += fmt::format("<{0}> {0}", formatNumber(jumpCase.first));
					}
					if (!options.empty())
						options += " | ";
					options += "<default> default";
					m_result << fmt::format("Block{}_{}Exit [label=\"{{ JT | {{ {} }} }}\" shape=Mrecord];\n", _functionIndex, _id.value, options);
					for (auto const& jumpCase: jt.cases)
						m_result << fmt::format("Block{}_{}Exit:{} -> Block{}_{};\n", _functionIndex, _id.value, formatNumber(jumpCase.first), _functionIndex, jumpCase.second.value);
					m_result << fmt::format("Block{}_{}Exit:default -> Block{}_{};\n", _functionIndex, _id.value, _functionIndex, jt.defaultCase.value);
				},
				[&](SSACFG::BasicBlock::FunctionReturn const& fr)
				{
					m_result << "Block" << _functionIndex << "_" << _id.value << "Exit [label=\"FunctionReturn["
							<< fmt::format("{}", fmt::join(fr.returnValues | ranges::views::transform(valueToString), ", "))
							<< "]\"];\n";
					m_result << "Block" << _functionIndex << "_" << _id.value << " -> Block" << _functionIndex << "_" << _id.value << "Exit;\n";
				},
				[&](SSACFG::BasicBlock::Terminated const&)
				{
					m_result << "Block" << _functionIndex << "_" << _id.value << "Exit [label=\"Terminated\"];\n";
					m_result << "Block" << _functionIndex << "_" << _id.value << " -> Block" << _functionIndex << "_" << _id.value << "Exit;\n";
				}
			}, _block.exit);
		}
	}

	void printBlock(SSACFG const& _cfg, SSACFG::BlockId const& _rootId, size_t const _functionIndex)
	{
		std::set<SSACFG::BlockId> explored{};
		explored.insert(_rootId);

		std::deque<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(_rootId);

		while (!toVisit.empty())
		{
			auto const id = toVisit.front();
			toVisit.pop_front();
			auto const& block = _cfg.block(id);
			writeBlock(_cfg, id, block, _functionIndex);
			block.forEachExit(
				[&](SSACFG::BlockId const& _exitBlock)
				{
					if (explored.count(_exitBlock) == 0)
					{
						explored.insert(_exitBlock);
						toVisit.emplace_back(_exitBlock);
					}
				}
			);
		}
	}

	void printFunction(SSACFG const& _cfg, size_t const _functionIndex, Scope::Function const& _fun)
	{
		static auto constexpr returnsTransform = [](auto const& functionReturnValue) { return functionReturnValue.get().name.str(); };
		static auto constexpr argsTransform = [](auto const& arg) { return fmt::format("v{}", std::get<1>(arg).value); };
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << _cfg.entry.value << " [label=\"";
		if (!_cfg.returns.empty())
			m_result << fmt::format("function {0}:\n {1} := {0}({2})", _fun.name.str(), fmt::join(_cfg.returns | ranges::views::transform(returnsTransform), ", "), fmt::join(_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		else
			m_result << fmt::format("function {0}:\n {0}({1})", _fun.name.str(), fmt::join(_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		m_result << "\"];\n";
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << _cfg.entry.value << " -> Block" << _functionIndex << "_" << _cfg.entry.value << ";\n";
		printBlock(_cfg, _cfg.entry, _functionIndex);
	}

	std::stringstream m_result{};
};
}

std::string SSACFG::toDot(bool _includeDiGraphDefinition, std::optional<size_t> _functionIndex) const
{
	std::ostringstream output;
	if (_includeDiGraphDefinition)
		output << "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\"]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";
	if (function)
		output << SSACFGPrinter(*this, _functionIndex ? *_functionIndex : static_cast<size_t>(1), *function);
	else
		output << SSACFGPrinter(*this, entry);
	if (_includeDiGraphDefinition)
		output << "}\n";
	return output.str();
}
