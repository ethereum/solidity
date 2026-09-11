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

#include <libyul/backends/evm/ssa/io/Printer.h>

#include <libyul/backends/evm/ssa/LivenessAnalysis.h>
#include <libyul/backends/evm/ssa/SSACFG.h>

#include <libsolutil/Numeric.h>

#include <optional>

namespace solidity::yul::ssa
{

struct ControlFlowGraphs;

struct ControlFlowGraphsLiveness{
	explicit ControlFlowGraphsLiveness(ControlFlowGraphs const& _controlFlow);

	std::reference_wrapper<ControlFlowGraphs const> controlFlowGraphs;
	std::vector<std::unique_ptr<LivenessAnalysis>> cfgLiveness;

	std::string toDot() const;
};

struct ControlFlowGraphs
{
	using FunctionGraphID = ssa::FunctionGraphID;

	static FunctionGraphID constexpr mainGraphID() noexcept { return 0; }

	SSACFG const* mainGraph() const { return functionGraph(mainGraphID()); }

	SSACFG const* functionGraph(FunctionGraphID const _id) const
	{
		return functionGraphs.at(_id).get();
	}

	std::string toDot(ControlFlowGraphsLiveness const* _liveness=nullptr) const
	{
		if (_liveness)
			yulAssert(&_liveness->controlFlowGraphs.get() == this);
		std::ostringstream output;
		output << "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\"]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";

		for (size_t index=0; index < functionGraphs.size(); ++index)
			output << functionGraphs[index]->toDot(
				false,
				index,
				_liveness ? _liveness->cfgLiveness[index].get() : nullptr,
				this
			);

		output << "}\n";
		return output.str();
	}

#ifdef SLOW_DEBUG
	void checkInvariants() const;
#endif

	std::string print() const
	{
		std::ostringstream output;
		io::print(output, *this);
		return output.str();
	}

	/// Memoryguard boundary for this subobject.
	/// - empty: no `memoryguard(N)` call appears in the source
	/// - set: every MemoryGuard Inst lowers to a `pushN <value>` of this constant
	std::optional<u256> memoryGuard{};

	std::vector<std::unique_ptr<SSACFG>> functionGraphs{};
};

}
