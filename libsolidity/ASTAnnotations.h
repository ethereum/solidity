/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Object containing the type and other annotations for the AST nodes.
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

namespace dev
{
namespace solidity
{

class ASTNode;
class ContractDefinition;
class Declaration;
class ParameterList;
class Type;
using TypePointer = std::shared_ptr<Type const>;

struct ASTAnnotation
{
	///@TODO save space here - we do not need all members for all types.

	/// For expression: Inferred type. - For type declaration: Declared type. - For variable declaration: Type of variable.
	TypePointer type;
	/// For expression: Whether it is an LValue (i.e. something that can be assigned to).
	bool isLValue = false;
	/// For expression: Whether the expression is used in a context where the LValue is actually required.
	bool lValueRequested = false;
	/// For expressions: Types of arguments if the expression is a function that is called - used
	/// for overload resolution.
	std::shared_ptr<std::vector<TypePointer>> argumentTypes;
	/// For contract: Whether all functions are implemented.
	bool isFullyImplemented = true;
	/// For contract: List of all (direct and indirect) base contracts in order from derived to
	/// base, including the contract itself.
	std::vector<ContractDefinition const*> linearizedBaseContracts;
	/// For member access and Identifer: Referenced declaration, set during overload resolution stage.
	Declaration const* referencedDeclaration = nullptr;
	/// For Identifier: List of possible declarations it could refer to.
	std::vector<Declaration const*> overloadedDeclarations;
	/// For function call: Whether this is an explicit type conversion.
	bool isTypeConversion = false;
	/// For function call: Whether this is a struct constructor call.
	bool isStructConstructorCall = false;
	/// For Return statement: Reference to the return parameters of the function.
	ParameterList const* functionReturnParameters = nullptr;
	/// For Identifier: Stores a reference to the current contract.
	/// This is needed because types of base contracts change depending on the context.
	ContractDefinition const* contractScope = nullptr;
	/// For BinaryOperation: The common type that is used for the operation, not necessarily the result type (e.g. for
	/// comparisons, this is always bool).
	TypePointer commonType;
};

}
}
