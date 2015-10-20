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
#include <set>
#include <libsolidity/ast/ASTForward.h>

namespace dev
{
namespace solidity
{

class Type;
using TypePointer = std::shared_ptr<Type const>;

struct ASTAnnotation
{
	virtual ~ASTAnnotation() {}
};

struct TypeDeclarationAnnotation: ASTAnnotation
{
	/// The name of this type, prefixed by proper namespaces if globally accessible.
	std::string canonicalName;
};

struct ContractDefinitionAnnotation: TypeDeclarationAnnotation
{
	/// Whether all functions are implemented.
	bool isFullyImplemented = true;
	/// List of all (direct and indirect) base contracts in order from derived to
	/// base, including the contract itself.
	std::vector<ContractDefinition const*> linearizedBaseContracts;
	/// List of contracts this contract creates, i.e. which need to be compiled first.
	/// Also includes all contracts from @a linearizedBaseContracts.
	std::set<ContractDefinition const*> contractDependencies;
};

struct VariableDeclarationAnnotation: ASTAnnotation
{
	/// Type of variable (type of identifier referencing this variable).
	TypePointer type;
};

struct ReturnAnnotation: ASTAnnotation
{
	/// Reference to the return parameters of the function.
	ParameterList const* functionReturnParameters = nullptr;
};

struct TypeNameAnnotation: ASTAnnotation
{
	/// Type declared by this type name, i.e. type of a variable where this type name is used.
	/// Set during reference resolution stage.
	TypePointer type;
};

struct UserDefinedTypeNameAnnotation: TypeNameAnnotation
{
	/// Referenced declaration, set during reference resolution stage.
	Declaration const* referencedDeclaration = nullptr;
};

struct VariableDeclarationStatementAnnotation: ASTAnnotation
{
	/// Information about which component of the value is assigned to which variable.
	/// The pointer can be null to signify that the component is discarded.
	std::vector<VariableDeclaration const*> assignments;
};

struct ExpressionAnnotation: ASTAnnotation
{
	/// Inferred type of the expression.
	TypePointer type;
	/// Whether it is an LValue (i.e. something that can be assigned to).
	bool isLValue = false;
	/// Whether the expression is used in a context where the LValue is actually required.
	bool lValueRequested = false;
	/// Types of arguments if the expression is a function that is called - used
	/// for overload resolution.
	std::shared_ptr<std::vector<TypePointer>> argumentTypes;
};

struct IdentifierAnnotation: ExpressionAnnotation
{
	/// Stores a reference to the current contract.
	/// This is needed because types of base contracts change depending on the context.
	ContractDefinition const* contractScope = nullptr;
	/// Referenced declaration, set at latest during overload resolution stage.
	Declaration const* referencedDeclaration = nullptr;
	/// List of possible declarations it could refer to.
	std::vector<Declaration const*> overloadedDeclarations;
};

struct MemberAccessAnnotation: ExpressionAnnotation
{
	/// Referenced declaration, set at latest during overload resolution stage.
	Declaration const* referencedDeclaration = nullptr;
};

struct BinaryOperationAnnotation: ExpressionAnnotation
{
	/// The common type that is used for the operation, not necessarily the result type (which
	/// e.g. for comparisons is bool).
	TypePointer commonType;
};

struct FunctionCallAnnotation: ExpressionAnnotation
{
	/// Whether this is an explicit type conversion.
	bool isTypeConversion = false;
	/// Whether this is a struct constructor call.
	bool isStructConstructorCall = false;
};

}
}
