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

#include <libyul/AST.h>
#include <libyul/Scope.h>
#include <libyul/backends/evm/SSACFGLiveness.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>

namespace solidity::yul
{

struct ControlFlow;

struct ControlFlowLiveness{
	explicit ControlFlowLiveness(ControlFlow const& _controlFlow);

	std::reference_wrapper<ControlFlow const> controlFlow;
	std::unique_ptr<SSACFGLiveness> mainLiveness;
	std::vector<std::unique_ptr<SSACFGLiveness>> functionLiveness;
};

struct ControlFlow
{
	std::unique_ptr<SSACFG> mainGraph{std::make_unique<SSACFG>()};
	std::vector<std::unique_ptr<SSACFG>> functionGraphs{};
	std::vector<std::tuple<Scope::Function const*, SSACFG const*>> functionGraphMapping{};

	SSACFG const* functionGraph(Scope::Function const* _function)
	{
		auto it = std::find_if(functionGraphMapping.begin(), functionGraphMapping.end(), [_function](auto const& tup) { return _function == std::get<0>(tup); });
		if (it != functionGraphMapping.end())
			return std::get<1>(*it);
		return nullptr;
	}

	std::string toDot(ControlFlowLiveness const* _liveness=nullptr) const
	{
		if (_liveness)
			yulAssert(&_liveness->controlFlow.get() == this);
		std::ostringstream output;
		output << "digraph SSACFG {\nnodesep=0.7;\ngraph[fontname=\"DejaVu Sans\"]\nnode[shape=box,fontname=\"DejaVu Sans\"];\n\n";
		output << mainGraph->toDot(false, std::nullopt, _liveness ? _liveness->mainLiveness.get() : nullptr);

		for (size_t index=0; index < functionGraphs.size(); ++index)
			output << functionGraphs[index]->toDot(
				false,
				index+1,
				_liveness ? _liveness->functionLiveness[index].get() : nullptr
			);

		output << "}\n";
		return output.str();
	}
};

}
