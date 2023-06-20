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

#include <libsolidity/ast/ASTForward.h>

#include <list>
#include <set>

namespace solidity::frontend::experimental
{

class Analysis;

struct IRGenerationContext
{
	Analysis const& analysis;
	std::list<FunctionDefinition const*> functionQueue;
	std::set<FunctionDefinition const*> generatedFunctions;
	void enqueueFunctionDefinition(FunctionDefinition const* _functionDefinition)
	{
		if (!generatedFunctions.count(_functionDefinition))
			functionQueue.emplace_back(_functionDefinition);
	}
};

}
