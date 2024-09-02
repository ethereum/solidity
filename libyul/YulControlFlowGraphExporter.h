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

#include <libyul/backends/evm/ControlFlow.h>
#include <libsolutil/JSON.h>
#include <libsolutil/Visitor.h>

using namespace solidity;
using namespace yul;

class YulControlFlowGraphExporter
{
public:
	YulControlFlowGraphExporter(ControlFlow const& _controlFlow);
	Json run();
	Json exportBlock(SSACFG const& _cfg, SSACFG::BlockId _blockId);
	Json exportFunction(SSACFG const& _cfg);
	std::string varToString(SSACFG const& _cfg, SSACFG::ValueId _var);

private:
	ControlFlow const& m_controlFlow;
	Json toJson(SSACFG const& _cfg, SSACFG::BlockId _blockId);
	Json toJson(Json& _ret, SSACFG const& _cfg, SSACFG::Operation const& _operation);
	Json toJson(SSACFG const& _cfg, std::vector<SSACFG::ValueId> const& _values);
};
