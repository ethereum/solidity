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

#pragma once

#include <libsolidity/ast/AST.h>

#include <algorithm>
#include <string>

namespace solidity::frontend
{

/**
 * Structure that describes arity and co-arity of a function, i.e. the number of its inputs and outputs.
 */
struct Arity
{
	size_t in;  /// Number of input parameters
	size_t out; /// Number of output parameters

	bool operator==(Arity const& _other) const { return in == _other.in && out == _other.out; }
	bool operator!=(Arity const& _other) const { return !(*this == _other); }
};

Arity getFunctionArity(FunctionDefinition const& _function);
Arity getFunctionArity(FunctionType const& _functionType);

std::string buildFunctionName(FunctionDefinition const& _function);
std::string buildFunctionName(VariableDeclaration const& _varDecl);
std::string buildCreationObjectName(ContractDefinition const& _contract);
std::string buildRuntimeObjectName(ContractDefinition const& _contract);
std::string buildInternalDispatchFunctionName(Arity const& _arity);

FunctionDefinition const* getReferencedFunctionDeclaration(Expression const& _expression);

}

// Overloading std::less() makes it possible to use Arity as a map key. We could define operator<
// instead but that would be confusing since e.g. Arity{2, 2} would be greater than Arity{1, 10}.
template<>
struct std::less<solidity::frontend::Arity>
{
	bool operator() (solidity::frontend::Arity const& _lhs, solidity::frontend::Arity const& _rhs) const
	{
		return _lhs.in < _rhs.in || (_lhs.in == _rhs.in && _lhs.out < _rhs.out);
	}
};
