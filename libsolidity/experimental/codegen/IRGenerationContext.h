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

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/ast/TypeSystem.h>

#include <list>
#include <set>

namespace solidity::frontend::experimental
{

class Analysis;

struct IRGenerationContext
{
	Analysis const& analysis;
	TypeEnvironment const* env = nullptr;
	void enqueueFunctionDefinition(FunctionDefinition const* _functionDefinition, Type _type)
	{
		QueuedFunction queue{_functionDefinition, env->resolve(_type)};
		for (auto type: generatedFunctions[_functionDefinition])
			if (env->typeEquals(type, _type))
				return;
		functionQueue.emplace_back(queue);
	}
	struct QueuedFunction
	{
		FunctionDefinition const* function = nullptr;
		Type type = std::monostate{};
	};
	std::list<QueuedFunction> functionQueue;
	std::map<FunctionDefinition const*, std::vector<Type>> generatedFunctions;
};

}
