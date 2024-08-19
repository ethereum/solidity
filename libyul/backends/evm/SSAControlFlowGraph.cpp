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

#include <libyul/backends/evm/SSACFGLiveness.h>

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
	SSACFGPrinter(SSACFG const& _cfg, SSACFG::BlockId _blockId, SSACFGLiveness const* _liveness):
		m_cfg(_cfg), m_functionIndex(0), m_liveness(_liveness)
	{
		printBlock(_blockId);
	}
	SSACFGPrinter(SSACFG const& _cfg, size_t _functionIndex, Scope::Function const& _function, SSACFGLiveness const* _liveness):
		m_cfg(_cfg), m_functionIndex(_functionIndex), m_liveness(_liveness)
	{
		printFunction(_function);
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

	std::string formatBlockHandle(SSACFG::BlockId const& _id) const
	{
		return fmt::format("Block{}_{}", m_functionIndex, _id.value);
	}

	std::string formatEdge(SSACFG::BlockId const& _v, SSACFG::BlockId const& _w, std::optional<std::string> const& _vPort = std::nullopt)
	{
		std::string const style = m_liveness && m_liveness->topologicalSort().backEdge(_v, _w) ? "dashed" : "solid";
		if (_vPort)
			return fmt::format("{}Exit:{} -> {} [style=\"{}\"];\n", formatBlockHandle(_v), *_vPort, formatBlockHandle(_w), style);
		else
			return fmt::format("{}Exit -> {} [style=\"{}\"];\n", formatBlockHandle(_v), formatBlockHandle(_w), style);
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

	void writeBlock(SSACFG::BlockId const& _id, SSACFG::BasicBlock const& _block)
	{
		auto const valueToString = [&](SSACFG::ValueId const& valueId) { return varToString(m_cfg, valueId); };
		bool entryBlock = _id.value == 0 && m_functionIndex == 0;
		if (entryBlock)
		{
			m_result << fmt::format("Entry{} [label=\"Entry\"];\n", m_functionIndex);
			m_result << fmt::format("Entry{} -> {};\n", m_functionIndex, formatBlockHandle(_id));
		}
		{
			if (m_liveness)
			{
				m_result << fmt::format(
					"{} [label=\"\\\nBlock {}; ({}, max {})\\n",
					formatBlockHandle(_id),
					_id.value,
					m_liveness->topologicalSort().preOrderIndexOf(_id.value),
					m_liveness->topologicalSort().maxSubtreePreOrderIndexOf(_id.value)
				);
				m_result << fmt::format(
					"LiveIn: {}\\l\\\n",
					fmt::join(m_liveness->liveIn(_id) | ranges::views::transform(valueToString), ",")
				);
				m_result << fmt::format(
					"LiveOut: {}\\l\\n",
					fmt::join(m_liveness->liveOut(_id) | ranges::views::transform(valueToString), ",")
				);
			}
			else
				m_result << fmt::format("{} [label=\"\\\nBlock {}\\n", formatBlockHandle(_id), _id.value);
			for (auto const& phi: _block.phis)
			{
				auto const* phiValue = std::get_if<SSACFG::PhiValue>(&m_cfg.valueInfo(phi));
				solAssert(phiValue);
				m_result << fmt::format("v{} := {}\\l\\\n", phi.value, formatPhi(m_cfg, *phiValue));
			}
			for (auto const& operation: _block.operations)
			{
				std::string const label = std::visit(GenericVisitor{
					[&](SSACFG::Call const& _call) {
						return _call.function.get().name.str();
					},
					[&](SSACFG::BuiltinCall const& _call) {
						return _call.builtin.get().name;
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
					m_result << fmt::format("{}Exit [label=\"MainExit\"];\n", formatBlockHandle(_id));
					m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
				},
				[&](SSACFG::BasicBlock::Jump const& _jump)
				{
					m_result << fmt::format("{} -> {}Exit [arrowhead=none];\n", formatBlockHandle(_id), formatBlockHandle(_id));
					m_result << fmt::format("{}Exit [label=\"Jump\" shape=oval];\n", formatBlockHandle(_id));
					m_result << formatEdge(_id, _jump.target);
				},
				[&](SSACFG::BasicBlock::ConditionalJump const& _conditionalJump)
				{
					m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
					m_result << fmt::format(
						"{}Exit [label=\"{{ If {} | {{ <0> Zero | <1> NonZero }}}}\" shape=Mrecord];\n",
						formatBlockHandle(_id), varToString(m_cfg, _conditionalJump.condition)
					);
					m_result << formatEdge(_id, _conditionalJump.zero, "0");
					m_result << formatEdge(_id, _conditionalJump.nonZero, "1");
				},
				[&](SSACFG::BasicBlock::JumpTable const& jt)
				{
					m_result << fmt::format("{} -> {}Exit;\n", formatBlockHandle(_id), formatBlockHandle(_id));
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
					m_result << fmt::format("{}Exit [label=\"{{ JT | {{ {} }} }}\" shape=Mrecord];\n", formatBlockHandle(_id), options);
					for (auto const& jumpCase: jt.cases)
						m_result << formatEdge(_id, jumpCase.second, formatNumber(jumpCase.first));
					m_result << formatEdge(_id, jt.defaultCase, "default");
				},
				[&](SSACFG::BasicBlock::FunctionReturn const& fr)
				{
					m_result << formatBlockHandle(_id) << "Exit [label=\"FunctionReturn["
							<< fmt::format("{}", fmt::join(fr.returnValues | ranges::views::transform(valueToString), ", "))
							<< "]\"];\n";
					m_result << formatBlockHandle(_id) << " -> " << formatBlockHandle(_id) << "Exit;\n";
				},
				[&](SSACFG::BasicBlock::Terminated const&)
				{
					m_result << formatBlockHandle(_id) << "Exit [label=\"Terminated\"];\n";
					m_result << formatBlockHandle(_id) << " -> " << formatBlockHandle(_id) << "Exit;\n";
				}
			}, _block.exit);
		}
	}

	void printBlock(SSACFG::BlockId const& _rootId)
	{
		std::set<SSACFG::BlockId> explored{};
		explored.insert(_rootId);

		std::deque<SSACFG::BlockId> toVisit{};
		toVisit.emplace_back(_rootId);

		while (!toVisit.empty())
		{
			auto const id = toVisit.front();
			toVisit.pop_front();
			auto const& block = m_cfg.block(id);
			writeBlock(id, block);
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

	void printFunction(Scope::Function const& _fun)
	{
		static auto constexpr returnsTransform = [](auto const& functionReturnValue) { return functionReturnValue.get().name.str(); };
		static auto constexpr argsTransform = [](auto const& arg) { return fmt::format("v{}", std::get<1>(arg).value); };
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << m_cfg.entry.value << " [label=\"";
		if (!m_cfg.returns.empty())
			m_result << fmt::format("function {0}:\n {1} := {0}({2})", _fun.name.str(), fmt::join(m_cfg.returns | ranges::views::transform(returnsTransform), ", "), fmt::join(m_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		else
			m_result << fmt::format("function {0}:\n {0}({1})", _fun.name.str(), fmt::join(m_cfg.arguments | ranges::views::transform(argsTransform), ", "));
		m_result << "\"];\n";
		m_result << "FunctionEntry_" << _fun.name.str() << "_" << m_cfg.entry.value << " -> Block" << m_functionIndex << "_" << m_cfg.entry.value << ";\n";
		printBlock(m_cfg.entry);
	}

	SSACFG const& m_cfg;
	size_t m_functionIndex;
	SSACFGLiveness const* m_liveness;
	std::stringstream m_result{};
};
}

std::string SSACFG::toDot(
	bool _includeDiGraphDefinition,
	std::optional<size_t> _functionIndex,
	SSACFGLiveness const* _liveness
) const
{
	std::ostringstream output;
	if (_includeDiGraphDefinition)
		output << "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\"]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";
	if (function)
		output << SSACFGPrinter(*this, _functionIndex ? *_functionIndex : static_cast<size_t>(1), *function, _liveness);
	else
		output << SSACFGPrinter(*this, entry, _liveness);
	if (_includeDiGraphDefinition)
		output << "}\n";
	return output.str();
}
