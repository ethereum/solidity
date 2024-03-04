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

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/codegen/ir/Common.h>
#include <libsolidity/codegen/ir/IRGenerationContext.h>

#include <libsolutil/CommonIO.h>

#include <libyul/AsmPrinter.h>

using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::util;
using namespace solidity::yul;

namespace solidity::frontend
{

YulArity YulArity::fromType(FunctionType const& _functionType)
{
	return YulArity{
		TupleType(_functionType.parameterTypesIncludingSelf()).sizeOnStack(),
		TupleType(_functionType.returnParameterTypes()).sizeOnStack()
	};
}

std::string IRNames::externalFunctionABIWrapper(Declaration const& _functionOrVarDecl)
{
	if (auto const* function = dynamic_cast<FunctionDefinition const*>(&_functionOrVarDecl))
		solAssert(!function->isConstructor());

	return "external_fun_" + _functionOrVarDecl.name() + "_" + std::to_string(_functionOrVarDecl.id());
}

std::string IRNames::function(FunctionDefinition const& _function)
{
	if (_function.isConstructor())
		return constructor(*_function.annotation().contract);

	return "fun_" + _function.name() + "_" + std::to_string(_function.id());
}

std::string IRNames::function(VariableDeclaration const& _varDecl)
{
	return "getter_fun_" + _varDecl.name() + "_" + std::to_string(_varDecl.id());
}

std::string IRNames::modifierInvocation(ModifierInvocation const& _modifierInvocation)
{
	// This uses the ID of the modifier invocation because it has to be unique
	// for each invocation.
	solAssert(!_modifierInvocation.name().path().empty(), "");
	std::string const& modifierName = _modifierInvocation.name().path().back();
	solAssert(!modifierName.empty(), "");
	return "modifier_" + modifierName + "_" + std::to_string(_modifierInvocation.id());
}

std::string IRNames::functionWithModifierInner(FunctionDefinition const& _function)
{
	return "fun_" + _function.name() + "_" + std::to_string(_function.id()) + "_inner";
}

std::string IRNames::creationObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id());
}

std::string IRNames::deployedObject(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + toString(_contract.id()) + "_deployed";
}

std::string IRNames::internalDispatch(YulArity const& _arity)
{
	return "dispatch_internal"
		"_in_" + std::to_string(_arity.in) +
		"_out_" + std::to_string(_arity.out);
}

std::string IRNames::constructor(ContractDefinition const& _contract)
{
	return "constructor_" + _contract.name() + "_" + std::to_string(_contract.id());
}

std::string IRNames::libraryAddressImmutable()
{
	return "library_deploy_address";
}

std::string IRNames::constantValueFunction(VariableDeclaration const& _constant)
{
	solAssert(_constant.isConstant(), "");
	return "constant_" + _constant.name() + "_" + std::to_string(_constant.id());
}

std::string IRNames::localVariable(VariableDeclaration const& _declaration)
{
	return "var_" + _declaration.name() + '_' + std::to_string(_declaration.id());
}

std::string IRNames::localVariable(Expression const& _expression)
{
	return "expr_" + std::to_string(_expression.id());
}

std::string IRNames::trySuccessConditionVariable(Expression const& _expression)
{
	auto annotation = dynamic_cast<FunctionCallAnnotation const*>(&_expression.annotation());
	solAssert(annotation, "");
	solAssert(annotation->tryCall, "Parameter must be a FunctionCall with tryCall-annotation set.");

	return "trySuccessCondition_" + std::to_string(_expression.id());
}

std::string IRNames::tupleComponent(size_t _i)
{
	return "component_" + std::to_string(_i + 1);
}

std::string IRNames::zeroValue(Type const& _type, std::string const& _variableName)
{
	return "zero_" + _type.identifier() + _variableName;
}

std::string dispenseLocationComment(langutil::SourceLocation const& _location, IRGenerationContext& _context)
{
	solAssert(_location.sourceName, "");
	_context.markSourceUsed(*_location.sourceName);

	std::string debugInfo = AsmPrinter::formatSourceLocation(
		_location,
		_context.sourceIndices(),
		_context.debugInfoSelection(),
		_context.soliditySourceProvider()
	);

	return debugInfo.empty() ? "" : "/// " + debugInfo;
}

std::string dispenseLocationComment(ASTNode const& _node, IRGenerationContext& _context)
{
	return dispenseLocationComment(_node.location(), _context);
}

}
