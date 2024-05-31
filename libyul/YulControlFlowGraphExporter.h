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

#include <libyul/backends/evm/ControlFlowGraph.h>
#include <libsolutil/JSON.h>
#include <libsolutil/Visitor.h>

using namespace solidity;
using namespace yul;

class YulControlFlowGraphExporter
{
public:
	YulControlFlowGraphExporter(){}
	void operator()(CFG::BasicBlock const& _block);

private:
	size_t getBlockId(CFG::BasicBlock const& _block);
	Json toJson(CFG::BasicBlock const& _entry);
	Json toJson(CFG::Operation const& _operation);
	Json stackToJson(Stack const& _stack);
	Json stackSlotToJson(StackSlot const& _slot);

	std::map<CFG::BasicBlock const*, size_t> m_blockIds;
	size_t m_blockCount = 0;
};
