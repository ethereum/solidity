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

#include <libsolidity/experimental/ast/CallGraph.h>

using namespace solidity::frontend::experimental;

bool CallGraph::CompareByID::operator()(FunctionDefinition const* _lhs, FunctionDefinition const* _rhs) const
{
	solAssert(_lhs && _rhs);
	return _lhs->id() < _rhs->id();
}

bool CallGraph::CompareByID::operator()(FunctionDefinition const* _lhs, int64_t _rhs) const
{
	solAssert(_lhs);
	return _lhs->id() < _rhs;
}

bool CallGraph::CompareByID::operator()(int64_t _lhs, FunctionDefinition const* _rhs) const
{
	solAssert(_rhs);
	return _lhs < _rhs->id();
}
