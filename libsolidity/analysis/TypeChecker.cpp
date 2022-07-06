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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Type analyzer and checker.
 */

#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Views.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/zip.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;

bool TypeChecker::typeSupportedByOldABIEncoder(Type const& _type, bool _isLibraryCall)
{
	if (_isLibraryCall && _type.dataStoredIn(DataLocation::Storage))
		return true;
	if (_type.category() == Type::Category::Struct)
		return false;
	if (_type.category() == Type::Category::Array)
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(_type);
		auto base = arrayType.baseType();
		if (!typeSupportedByOldABIEncoder(*base, _isLibraryCall) || (base->category() == Type::Category::Array && base->isDynamicallySized()))
			return false;
	}
	return true;
}

bool TypeChecker::checkTypeRequirements(SourceUnit const& _source)
{
	m_currentSourceUnit = &_source;
	_source.accept(*this);
	m_currentSourceUnit = nullptr;
	return !Error::containsErrors(m_errorReporter.errors());
}

Type const* TypeChecker::type(Expression const& _expression) const
{
	solAssert(!!_expression.annotation().type, "Type requested but not present.");
	return _expression.annotation().type;
}

Type const* TypeChecker::type(VariableDeclaration const& _variable) const
{
	solAssert(!!_variable.annotation().type, "Type requested but not present.");
	return _variable.annotation().type;
}

bool TypeChecker::visit(ContractDefinition const& _contract)
{
	m_currentContract = &_contract;

	ASTNode::listAccept(_contract.baseContracts(), *this);

	for (auto const& n: _contract.subNodes())
		n->accept(*this);

	m_currentContract = nullptr;

	return false;
}

void TypeChecker::checkDoubleStorageAssignment(Assignment const& _assignment)
{
	size_t storageToStorageCopies = 0;
	size_t toStorageCopies = 0;
	size_t storageByteArrayPushes = 0;
	size_t storageByteAccesses = 0;
	auto count = [&](TupleExpression const& _lhs, TupleType const& _rhs, auto _recurse) -> void {
		TupleType const& lhsType = dynamic_cast<TupleType const&>(*type(_lhs));
		TupleExpression const* lhsResolved = dynamic_cast<TupleExpression const*>(resolveOuterUnaryTuples(&_lhs));

		if (lhsType.components().size() != _rhs.components().size() || lhsResolved->components().size() != _rhs.components().size())
		{
			solAssert(m_errorReporter.hasErrors(), "");
			return;
		}

		for (auto&& [index, componentType]: lhsType.components() | ranges::views::enumerate)
		{
			if (ReferenceType const* ref = dynamic_cast<ReferenceType const*>(componentType))
			{
				if (ref && ref->dataStoredIn(DataLocation::Storage) && !ref->isPointer())
				{
					toStorageCopies++;
					if (_rhs.components()[index]->dataStoredIn(DataLocation::Storage))
						storageToStorageCopies++;
				}
			}
			else if (FixedBytesType const* bytesType = dynamic_cast<FixedBytesType const*>(componentType))
			{
				if (bytesType && bytesType->numBytes() == 1)
				{
					if (FunctionCall const* lhsCall = dynamic_cast<FunctionCall const*>(resolveOuterUnaryTuples(lhsResolved->components().at(index).get())))
					{
						FunctionType const& callType = dynamic_cast<FunctionType const&>(*type(lhsCall->expression()));
						if (callType.kind() == FunctionType::Kind::ArrayPush)
						{
							ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*callType.selfType());
							if (arrayType.isByteArray() && arrayType.dataStoredIn(DataLocation::Storage))
							{
								++storageByteAccesses;
								++storageByteArrayPushes;
							}
						}
					}
					else if (IndexAccess const* indexAccess = dynamic_cast<IndexAccess const*>(resolveOuterUnaryTuples(lhsResolved->components().at(index).get())))
					{
						if (ArrayType const* arrayType = dynamic_cast<ArrayType const*>(type(indexAccess->baseExpression())))
							if (arrayType->isByteArray() && arrayType->dataStoredIn(DataLocation::Storage))
								++storageByteAccesses;
					}
				}
			}
			else if (TupleType const* tupleType = dynamic_cast<TupleType const*>(componentType))
				if (auto const* lhsNested = dynamic_cast<TupleExpression const*>(lhsResolved->components().at(index).get()))
					if (auto const* rhsNestedType = dynamic_cast<TupleType const*>(_rhs.components().at(index)))
						_recurse(
							*lhsNested,
							*rhsNestedType,
							_recurse
						);
		}
	};

	TupleExpression const* lhsTupleExpression = dynamic_cast<TupleExpression const*>(&_assignment.leftHandSide());
	if (!lhsTupleExpression)
	{
		solAssert(m_errorReporter.hasErrors());
		return;
	}
	count(
		*lhsTupleExpression,
		dynamic_cast<TupleType const&>(*type(_assignment.rightHandSide())),
		count
	);

	if (storageToStorageCopies >= 1 && toStorageCopies >= 2)
		m_errorReporter.warning(
			7238_error,
			_assignment.location(),
			"This assignment performs two copies to storage. Since storage copies do not first "
			"copy to a temporary location, one of them might be overwritten before the second "
			"is executed and thus may have unexpected effects. It is safer to perform the copies "
			"separately or assign to storage pointers first."
		);

	if (storageByteArrayPushes >= 1 && storageByteAccesses >= 2)
		m_errorReporter.warning(
			7239_error,
			_assignment.location(),
			"This assignment involves multiple accesses to a bytes array in storage while simultaneously enlarging it. "
			"When a bytes array is enlarged, it may transition from short storage layout to long storage layout, "
			"which invalidates all references to its elements. It is safer to only enlarge byte arrays in a single "
			"operation, one element at a time."
		);
}

TypePointers TypeChecker::typeCheckABIDecodeAndRetrieveReturnType(FunctionCall const& _functionCall, bool _abiEncoderV2)
{
	vector<ASTPointer<Expression const>> arguments = _functionCall.arguments();
	if (arguments.size() != 2)
		m_errorReporter.typeError(
			5782_error,
			_functionCall.location(),
			"This function takes two arguments, but " +
			toString(arguments.size()) +
			" were provided."
		);

	if (arguments.size() >= 1)
		if (
			!type(*arguments.front())->isImplicitlyConvertibleTo(*TypeProvider::bytesMemory()) &&
			!type(*arguments.front())->isImplicitlyConvertibleTo(*TypeProvider::bytesCalldata())
		)
			m_errorReporter.typeError(
				1956_error,
				arguments.front()->location(),
				"The first argument to \"abi.decode\" must be implicitly convertible to "
				"bytes memory or bytes calldata, but is of type " +
				type(*arguments.front())->humanReadableName() +
				"."
			);

	if (arguments.size() < 2)
		return {};

	// The following is a rather syntactic restriction, but we check it here anyway:
	// The second argument has to be a tuple expression containing type names.
	TupleExpression const* tupleExpression = dynamic_cast<TupleExpression const*>(arguments[1].get());
	if (!tupleExpression)
	{
		m_errorReporter.typeError(
			6444_error,
			arguments[1]->location(),
			"The second argument to \"abi.decode\" has to be a tuple of types."
		);
		return {};
	}

	TypePointers components;
	for (auto const& typeArgument: tupleExpression->components())
	{
		solAssert(typeArgument, "");
		if (TypeType const* argTypeType = dynamic_cast<TypeType const*>(type(*typeArgument)))
		{
			Type const* actualType = argTypeType->actualType();
			solAssert(actualType, "");
			// We force memory because the parser currently cannot handle
			// data locations. Furthermore, storage can be a little dangerous and
			// calldata is not really implemented anyway.
			actualType = TypeProvider::withLocationIfReference(DataLocation::Memory, actualType);
			// We force address payable for address types.
			if (actualType->category() == Type::Category::Address)
				actualType = TypeProvider::payableAddress();
			solAssert(
				!actualType->dataStoredIn(DataLocation::CallData) &&
				!actualType->dataStoredIn(DataLocation::Storage),
				""
			);
			if (!actualType->fullEncodingType(false, _abiEncoderV2, false))
				m_errorReporter.typeError(
					9611_error,
					typeArgument->location(),
					"Decoding type " + actualType->humanReadableName() + " not supported."
				);

			if (auto referenceType = dynamic_cast<ReferenceType const*>(actualType))
			{
				auto result = referenceType->validForLocation(referenceType->location());
				if (!result)
					m_errorReporter.typeError(
						6118_error,
						typeArgument->location(),
						result.message()
					);
			}

			components.push_back(actualType);
		}
		else
		{
			m_errorReporter.typeError(1039_error, typeArgument->location(), "Argument has to be a type name.");
			components.push_back(TypeProvider::emptyTuple());
		}
	}
	return components;
}

TypePointers TypeChecker::typeCheckMetaTypeFunctionAndRetrieveReturnType(FunctionCall const& _functionCall)
{
	vector<ASTPointer<Expression const>> arguments = _functionCall.arguments();
	if (arguments.size() != 1)
		m_errorReporter.fatalTypeError(
			8885_error,
			_functionCall.location(),
			"This function takes one argument, but " +
			toString(arguments.size()) +
			" were provided."
		);
	Type const* firstArgType = type(*arguments.front());

	bool wrongType = false;
	if (firstArgType->category() == Type::Category::TypeType)
	{
		TypeType const* typeTypePtr = dynamic_cast<TypeType const*>(firstArgType);
		Type::Category typeCategory = typeTypePtr->actualType()->category();
		if (auto const* contractType = dynamic_cast<ContractType const*>(typeTypePtr->actualType()))
			wrongType = contractType->isSuper();
		else if (
			typeCategory != Type::Category::Integer &&
			typeCategory != Type::Category::Enum
		)
			wrongType = true;
	}
	else
		wrongType = true;

	if (wrongType)
		m_errorReporter.fatalTypeError(
			4259_error,
			arguments.front()->location(),
			"Invalid type for argument in the function call. "
			"An enum type, contract type or an integer type is required, but " +
			type(*arguments.front())->humanReadableName() + " provided."
		);

	return {TypeProvider::meta(dynamic_cast<TypeType const&>(*firstArgType).actualType())};
}

bool TypeChecker::visit(ImportDirective const&)
{
	return false;
}

void TypeChecker::endVisit(InheritanceSpecifier const& _inheritance)
{
	auto base = dynamic_cast<ContractDefinition const*>(&dereference(_inheritance.name()));
	solAssert(base, "Base contract not available.");
	solAssert(m_currentContract, "");

	if (m_currentContract->isInterface() && !base->isInterface())
		m_errorReporter.typeError(6536_error, _inheritance.location(), "Interfaces can only inherit from other interfaces.");

	auto const& arguments = _inheritance.arguments();
	TypePointers parameterTypes;
	if (!base->isInterface())
		// Interfaces do not have constructors, so there are zero parameters.
		parameterTypes = ContractType(*base).newExpressionType()->parameterTypes();

	if (arguments)
	{
		if (parameterTypes.size() != arguments->size())
		{
			m_errorReporter.typeError(
				7927_error,
				_inheritance.location(),
				"Wrong argument count for constructor call: " +
				toString(arguments->size()) +
				" arguments given but expected " +
				toString(parameterTypes.size()) +
				". Remove parentheses if you do not want to provide arguments here."
			);
		}
		for (size_t i = 0; i < std::min(arguments->size(), parameterTypes.size()); ++i)
		{
			BoolResult result = type(*(*arguments)[i])->isImplicitlyConvertibleTo(*parameterTypes[i]);
			if (!result)
				m_errorReporter.typeErrorConcatenateDescriptions(
					9827_error,
					(*arguments)[i]->location(),
					"Invalid type for argument in constructor call. "
					"Invalid implicit conversion from " +
					type(*(*arguments)[i])->humanReadableName() +
					" to " +
					parameterTypes[i]->humanReadableName() +
					" requested.",
					result.message()
				);
		}
	}
}

void TypeChecker::endVisit(ModifierDefinition const& _modifier)
{
	if (auto const* contractDef = dynamic_cast<ContractDefinition const*>(_modifier.scope()))
	{
		if (_modifier.virtualSemantics() && contractDef->isLibrary())
			m_errorReporter.typeError(
				3275_error,
				_modifier.location(),
				"Modifiers in a library cannot be virtual."
			);

		if (contractDef->isInterface())
			m_errorReporter.typeError(
				6408_error,
				_modifier.location(),
				"Modifiers cannot be defined or declared in interfaces."
			);
	}

	if (!_modifier.isImplemented() && !_modifier.virtualSemantics())
		m_errorReporter.typeError(8063_error, _modifier.location(), "Modifiers without implementation must be marked virtual.");
}

bool TypeChecker::visit(FunctionDefinition const& _function)
{
	if (_function.markedVirtual())
	{
		if (_function.isFree())
			m_errorReporter.syntaxError(4493_error, _function.location(), "Free functions cannot be virtual.");
		else if (_function.isConstructor())
			m_errorReporter.typeError(7001_error, _function.location(), "Constructors cannot be virtual.");
		else if (_function.annotation().contract->isInterface())
			m_errorReporter.warning(5815_error, _function.location(), "Interface functions are implicitly \"virtual\"");
		else if (_function.visibility() == Visibility::Private)
			m_errorReporter.typeError(3942_error, _function.location(), "\"virtual\" and \"private\" cannot be used together.");
		else if (_function.libraryFunction())
			m_errorReporter.typeError(7801_error, _function.location(), "Library functions cannot be \"virtual\".");
	}
	if (_function.overrides() && _function.isFree())
		m_errorReporter.syntaxError(1750_error, _function.location(), "Free functions cannot override.");

	if (!_function.modifiers().empty() && _function.isFree())
		m_errorReporter.syntaxError(5811_error, _function.location(), "Free functions cannot have modifiers.");

	if (_function.isPayable())
	{
		if (_function.libraryFunction())
			m_errorReporter.typeError(7708_error, _function.location(), "Library functions cannot be payable.");
		else if (_function.isFree())
			m_errorReporter.typeError(9559_error, _function.location(), "Free functions cannot be payable.");
		else if (_function.isOrdinary() && !_function.isPartOfExternalInterface())
			m_errorReporter.typeError(5587_error, _function.location(), "\"internal\" and \"private\" functions cannot be payable.");
	}

	vector<VariableDeclaration const*> internalParametersInConstructor;

	auto checkArgumentAndReturnParameter = [&](VariableDeclaration const& _var) {
		if (type(_var)->containsNestedMapping())
			if (_var.referenceLocation() == VariableDeclaration::Location::Storage)
				solAssert(
					_function.libraryFunction() || _function.isConstructor() || !_function.isPublic(),
					"Mapping types for parameters or return variables "
					"can only be used in internal or library functions."
				);
		bool functionIsExternallyVisible =
			(!_function.isConstructor() && _function.isPublic()) ||
			(_function.isConstructor() && !m_currentContract->abstract());
		if (
			_function.isConstructor() &&
			_var.referenceLocation() == VariableDeclaration::Location::Storage &&
			!m_currentContract->abstract()
		)
			m_errorReporter.fatalTypeError(
				3644_error,
				_var.location(),
				"This parameter has a type that can only be used internally. "
				"You can make the contract abstract to avoid this problem."
			);
		else if (functionIsExternallyVisible)
		{
			auto iType = type(_var)->interfaceType(_function.libraryFunction());

			if (!iType)
			{
				string message = iType.message();
				solAssert(!message.empty(), "Expected detailed error message!");
				if (_function.isConstructor())
					message += " You can make the contract abstract to avoid this problem.";
				m_errorReporter.fatalTypeError(4103_error, _var.location(), message);
			}
			else if (
				!useABICoderV2() &&
				!typeSupportedByOldABIEncoder(*type(_var), _function.libraryFunction())
			)
			{
				string message =
					"This type is only supported in ABI coder v2. "
					"Use \"pragma abicoder v2;\" to enable the feature.";
				if (_function.isConstructor())
					message +=
						" Alternatively, make the contract abstract and supply the "
						"constructor arguments from a derived contract.";
				m_errorReporter.typeError(
					4957_error,
					_var.location(),
					message
				);
			}
		}
	};
	for (ASTPointer<VariableDeclaration> const& var: _function.parameters())
	{
		checkArgumentAndReturnParameter(*var);
		var->accept(*this);
	}
	for (ASTPointer<VariableDeclaration> const& var: _function.returnParameters())
	{
		checkArgumentAndReturnParameter(*var);
		var->accept(*this);
	}

	set<Declaration const*> modifiers;
	for (ASTPointer<ModifierInvocation> const& modifier: _function.modifiers())
	{
		vector<ContractDefinition const*> baseContracts;
		if (auto contract = dynamic_cast<ContractDefinition const*>(_function.scope()))
		{
			baseContracts = contract->annotation().linearizedBaseContracts;
			// Delete first base which is just the main contract itself
			baseContracts.erase(baseContracts.begin());
		}

		visitManually(
			*modifier,
			_function.isConstructor() ? baseContracts : vector<ContractDefinition const*>()
		);
		Declaration const* decl = &dereference(modifier->name());
		if (modifiers.count(decl))
		{
			if (dynamic_cast<ContractDefinition const*>(decl))
				m_errorReporter.declarationError(1697_error, modifier->location(), "Base constructor already provided.");
		}
		else
			modifiers.insert(decl);
	}

	solAssert(_function.isFree() == !m_currentContract, "");
	if (!m_currentContract)
	{
		solAssert(!_function.isConstructor(), "");
		solAssert(!_function.isFallback(), "");
		solAssert(!_function.isReceive(), "");
	}
	else if (m_currentContract->isInterface())
	{
		if (_function.isImplemented())
			m_errorReporter.typeError(4726_error, _function.location(), "Functions in interfaces cannot have an implementation.");

		if (_function.isConstructor())
			m_errorReporter.typeError(6482_error, _function.location(), "Constructor cannot be defined in interfaces.");
		else if (_function.visibility() != Visibility::External)
			m_errorReporter.typeError(1560_error, _function.location(), "Functions in interfaces must be declared external.");
	}
	else if (m_currentContract->contractKind() == ContractKind::Library)
		if (_function.isConstructor())
			m_errorReporter.typeError(7634_error, _function.location(), "Constructor cannot be defined in libraries.");

	if (_function.isImplemented())
		_function.body().accept(*this);
	else if (_function.isConstructor())
		m_errorReporter.typeError(5700_error, _function.location(), "Constructor must be implemented if declared.");
	else if (_function.libraryFunction())
		m_errorReporter.typeError(9231_error, _function.location(), "Library functions must be implemented if declared.");
	else if (!_function.virtualSemantics())
	{
		if (_function.isFree())
			solAssert(m_errorReporter.hasErrors(), "");
		else
			m_errorReporter.typeError(5424_error, _function.location(), "Functions without implementation must be marked virtual.");
	}


	if (_function.isFallback())
		typeCheckFallbackFunction(_function);
	else if (_function.isConstructor())
		typeCheckConstructor(_function);

	return false;
}

bool TypeChecker::visit(VariableDeclaration const& _variable)
{
	_variable.typeName().accept(*this);

	// type is filled either by ReferencesResolver directly from the type name or by
	// TypeChecker at the VariableDeclarationStatement level.
	Type const* varType = _variable.annotation().type;
	solAssert(!!varType, "Variable type not provided.");

	if (_variable.value())
	{
		if (_variable.isStateVariable() && varType->containsNestedMapping())
		{
			m_errorReporter.typeError(
				6280_error,
				_variable.location(),
				"Types in storage containing (nested) mappings cannot be assigned to."
			);
			_variable.value()->accept(*this);
		}
		else
			expectType(*_variable.value(), *varType);
	}
	if (_variable.isConstant())
	{
		if (!_variable.value())
			m_errorReporter.typeError(4266_error, _variable.location(), "Uninitialized \"constant\" variable.");
		else if (!*_variable.value()->annotation().isPure)
			m_errorReporter.typeError(
				8349_error,
				_variable.value()->location(),
				"Initial value for constant variable has to be compile-time constant."
			);
	}
	else if (_variable.immutable())
	{
		if (!_variable.type()->isValueType())
			m_errorReporter.typeError(6377_error, _variable.location(), "Immutable variables cannot have a non-value type.");
		if (
			auto const* functionType = dynamic_cast<FunctionType const*>(_variable.type());
			functionType && functionType->kind() == FunctionType::Kind::External
		)
			m_errorReporter.typeError(3366_error, _variable.location(), "Immutable variables of external function type are not yet supported.");
		solAssert(_variable.type()->sizeOnStack() == 1 || m_errorReporter.hasErrors(), "");
	}

	if (!_variable.isStateVariable())
	{
		if (
			_variable.referenceLocation() == VariableDeclaration::Location::CallData ||
			_variable.referenceLocation() == VariableDeclaration::Location::Memory
		)
			if (varType->containsNestedMapping())
				m_errorReporter.fatalTypeError(
					4061_error,
					_variable.location(),
					"Type " + varType->humanReadableName() + " is only valid in storage because it contains a (nested) mapping."
				);
	}
	else if (_variable.visibility() >= Visibility::Public)
	{
		FunctionType getter(_variable);
		if (!useABICoderV2())
		{
			vector<string> unsupportedTypes;
			for (auto const& param: getter.parameterTypes() + getter.returnParameterTypes())
				if (!typeSupportedByOldABIEncoder(*param, false /* isLibrary */))
					unsupportedTypes.emplace_back(param->humanReadableName());
			if (!unsupportedTypes.empty())
				m_errorReporter.typeError(
					2763_error,
					_variable.location(),
					"The following types are only supported for getters in ABI coder v2: " +
					joinHumanReadable(unsupportedTypes) +
					". Either remove \"public\" or use \"pragma abicoder v2;\" to enable the feature."
				);
		}
		if (!getter.interfaceFunctionType())
		{
			solAssert(getter.returnParameterNames().size() == getter.returnParameterTypes().size());
			solAssert(getter.parameterNames().size() == getter.parameterTypes().size());
			if (getter.returnParameterTypes().empty() && getter.parameterTypes().empty())
				m_errorReporter.typeError(5359_error, _variable.location(), "The struct has all its members omitted, therefore the getter cannot return any values.");
			else
				m_errorReporter.typeError(6744_error, _variable.location(), "Internal or recursive type is not allowed for public state variables.");
		}
	}

	bool isStructMemberDeclaration = dynamic_cast<StructDefinition const*>(_variable.scope()) != nullptr;
	if (isStructMemberDeclaration)
		return false;

	if (auto referenceType = dynamic_cast<ReferenceType const*>(varType))
	{
		BoolResult result = referenceType->validForLocation(referenceType->location());
		if (result)
		{
			bool isLibraryStorageParameter = (_variable.isLibraryFunctionParameter() && referenceType->location() == DataLocation::Storage);
			// We skip the calldata check for abstract contract constructors.
			bool isAbstractConstructorParam = _variable.isConstructorParameter() && m_currentContract && m_currentContract->abstract();
			bool callDataCheckRequired =
				!isAbstractConstructorParam &&
				(_variable.isConstructorParameter() || _variable.isPublicCallableParameter()) &&
				!isLibraryStorageParameter;
			if (callDataCheckRequired)
			{
				if (!referenceType->interfaceType(false))
					solAssert(m_errorReporter.hasErrors(), "");
				else
					result = referenceType->validForLocation(DataLocation::CallData);
			}
		}
		if (!result)
		{
			solAssert(!result.message().empty(), "Expected detailed error message");
			m_errorReporter.typeError(1534_error, _variable.location(), result.message());
			return false;
		}
	}

	return false;
}

void TypeChecker::endVisit(StructDefinition const& _struct)
{
	for (auto const& member: _struct.members())
		solAssert(
			member->annotation().type &&
			member->annotation().type->canBeStored(),
			"Type cannot be used in struct."
		);
}

void TypeChecker::visitManually(
	ModifierInvocation const& _modifier,
	vector<ContractDefinition const*> const& _bases
)
{
	std::vector<ASTPointer<Expression>> const& arguments =
		_modifier.arguments() ? *_modifier.arguments() : std::vector<ASTPointer<Expression>>();
	for (ASTPointer<Expression> const& argument: arguments)
		argument->accept(*this);

	_modifier.name().accept(*this);

	auto const* declaration = &dereference(_modifier.name());
	vector<ASTPointer<VariableDeclaration>> emptyParameterList;
	vector<ASTPointer<VariableDeclaration>> const* parameters = nullptr;
	if (auto modifierDecl = dynamic_cast<ModifierDefinition const*>(declaration))
	{
		parameters = &modifierDecl->parameters();
		if (auto const* modifierContract = dynamic_cast<ContractDefinition const*>(modifierDecl->scope()))
			if (m_currentContract)
			{
				if (!util::contains(m_currentContract->annotation().linearizedBaseContracts, modifierContract))
					m_errorReporter.typeError(
						9428_error,
						_modifier.location(),
						"Can only use modifiers defined in the current contract or in base contracts."
					);
			}
		if (
			*_modifier.name().annotation().requiredLookup == VirtualLookup::Static &&
			!modifierDecl->isImplemented()
		)
			m_errorReporter.typeError(
				1835_error,
				_modifier.location(),
				"Cannot call unimplemented modifier. The modifier has no implementation in the referenced contract. Refer to it by its unqualified name if you want to call the implementation from the most derived contract."
			);
	}
	else
		// check parameters for Base constructors
		for (ContractDefinition const* base: _bases)
			if (declaration == base)
			{
				if (auto referencedConstructor = base->constructor())
					parameters = &referencedConstructor->parameters();
				else
					parameters = &emptyParameterList;
				break;
			}
	if (!parameters)
	{
		m_errorReporter.typeError(4659_error, _modifier.location(), "Referenced declaration is neither modifier nor base class.");
		return;
	}
	if (parameters->size() != arguments.size())
	{
		m_errorReporter.typeError(
			2973_error,
			_modifier.location(),
			"Wrong argument count for modifier invocation: " +
			toString(arguments.size()) +
			" arguments given but expected " +
			toString(parameters->size()) +
			"."
		);
		return;
	}
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		BoolResult result = type(*arguments[i])->isImplicitlyConvertibleTo(*type(*(*parameters)[i]));
		if (!result)
			m_errorReporter.typeErrorConcatenateDescriptions(
				4649_error,
				arguments[i]->location(),
				"Invalid type for argument in modifier invocation. "
				"Invalid implicit conversion from " +
				type(*arguments[i])->humanReadableName() +
				" to " +
				type(*(*parameters)[i])->humanReadableName() +
				" requested.",
				result.message()
			);
	}
}

bool TypeChecker::visit(EventDefinition const& _eventDef)
{
	solAssert(_eventDef.visibility() > Visibility::Internal, "");
	checkErrorAndEventParameters(_eventDef);

	auto numIndexed = ranges::count_if(
		_eventDef.parameters(),
		[](ASTPointer<VariableDeclaration> const& var) { return var->isIndexed(); }
	);
	if (_eventDef.isAnonymous() && numIndexed > 4)
		m_errorReporter.typeError(8598_error, _eventDef.location(), "More than 4 indexed arguments for anonymous event.");
	else if (!_eventDef.isAnonymous() && numIndexed > 3)
		m_errorReporter.typeError(7249_error, _eventDef.location(), "More than 3 indexed arguments for event.");
	return true;
}

bool TypeChecker::visit(ErrorDefinition const& _errorDef)
{
	solAssert(_errorDef.visibility() > Visibility::Internal, "");
	checkErrorAndEventParameters(_errorDef);
	return true;
}

void TypeChecker::endVisit(FunctionTypeName const& _funType)
{
	FunctionType const& fun = dynamic_cast<FunctionType const&>(*_funType.annotation().type);
	if (fun.kind() == FunctionType::Kind::External)
	{
		for (auto const& t: _funType.parameterTypes() + _funType.returnParameterTypes())
		{
			solAssert(t->annotation().type, "Type not set for parameter.");
			if (!t->annotation().type->interfaceType(false).get())
				m_errorReporter.fatalTypeError(2582_error, t->location(), "Internal type cannot be used for external function type.");
		}
		solAssert(fun.interfaceType(false), "External function type uses internal types.");
	}
}

bool TypeChecker::visit(InlineAssembly const& _inlineAssembly)
{
	bool lvalueAccessToMemoryVariable = false;
	// External references have already been resolved in a prior stage and stored in the annotation.
	// We run the resolve step again regardless.
	yul::ExternalIdentifierAccess::Resolver identifierAccess = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext _context,
		bool
	) -> bool
	{
		if (_context == yul::IdentifierContext::NonExternal)
		{
			// Hack until we can disallow any shadowing: If we found an internal reference,
			// clear the external references, so that codegen does not use it.
			_inlineAssembly.annotation().externalReferences.erase(& _identifier);
			return false;
		}
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return false;
		InlineAssemblyAnnotation::ExternalIdentifierInfo& identifierInfo = ref->second;
		Declaration const* declaration = identifierInfo.declaration;
		solAssert(!!declaration, "");
		if (auto var = dynamic_cast<VariableDeclaration const*>(declaration))
		{
			solAssert(var->type(), "Expected variable type!");
			if (_context == yul::IdentifierContext::LValue && var->type()->dataStoredIn(DataLocation::Memory))
				lvalueAccessToMemoryVariable = true;
			if (var->immutable())
			{
				m_errorReporter.typeError(3773_error, nativeLocationOf(_identifier), "Assembly access to immutable variables is not supported.");
				return false;
			}
			if (var->isConstant())
			{
				if (isConstantVariableRecursive(*var))
				{
					m_errorReporter.typeError(
						3558_error,
						nativeLocationOf(_identifier),
						"Constant variable is circular."
					);
					return false;
				}

				var = rootConstVariableDeclaration(*var);

				if (var && !var->value())
				{
					m_errorReporter.typeError(3224_error, nativeLocationOf(_identifier), "Constant has no value.");
					return false;
				}
				else if (_context == yul::IdentifierContext::LValue)
				{
					m_errorReporter.typeError(6252_error, nativeLocationOf(_identifier), "Constant variables cannot be assigned to.");
					return false;
				}
				else if (identifierInfo.suffix == "slot" || identifierInfo.suffix == "offset")
				{
					m_errorReporter.typeError(6617_error, nativeLocationOf(_identifier), "The suffixes .offset and .slot can only be used on non-constant storage variables.");
					return false;
				}
				else if (var && var->value() && !var->value()->annotation().type && !dynamic_cast<Literal const*>(var->value().get()))
				{
					m_errorReporter.typeError(
						2249_error,
						nativeLocationOf(_identifier),
						"Constant variables with non-literal values cannot be forward referenced from inline assembly."
					);
					return false;
				}
				else if (!var || !type(*var)->isValueType() || (
					!dynamic_cast<Literal const*>(var->value().get()) &&
					type(*var->value())->category() != Type::Category::RationalNumber
				))
				{
					m_errorReporter.typeError(7615_error, nativeLocationOf(_identifier), "Only direct number constants and references to such constants are supported by inline assembly.");
					return false;
				}
			}

			solAssert(!dynamic_cast<FixedPointType const*>(var->type()), "FixedPointType not implemented.");

			if (!identifierInfo.suffix.empty())
			{
				string const& suffix = identifierInfo.suffix;
				solAssert((set<string>{"offset", "slot", "length", "selector", "address"}).count(suffix), "");
				if (!var->isConstant() && (var->isStateVariable() || var->type()->dataStoredIn(DataLocation::Storage)))
				{
					if (suffix != "slot" && suffix != "offset")
					{
						m_errorReporter.typeError(4656_error, nativeLocationOf(_identifier), "State variables only support \".slot\" and \".offset\".");
						return false;
					}
					else if (_context == yul::IdentifierContext::LValue)
					{
						if (var->isStateVariable())
						{
							m_errorReporter.typeError(4713_error, nativeLocationOf(_identifier), "State variables cannot be assigned to - you have to use \"sstore()\".");
							return false;
						}
						else if (suffix != "slot")
						{
							m_errorReporter.typeError(9739_error, nativeLocationOf(_identifier), "Only .slot can be assigned to.");
							return false;
						}
					}
				}
				else if (
					auto const* arrayType = dynamic_cast<ArrayType const*>(var->type());
					arrayType && arrayType->isDynamicallySized() && arrayType->dataStoredIn(DataLocation::CallData)
				)
				{
					if (suffix != "offset" && suffix != "length")
					{
						m_errorReporter.typeError(1536_error, nativeLocationOf(_identifier), "Calldata variables only support \".offset\" and \".length\".");
						return false;
					}
				}
				else if (auto const* fpType = dynamic_cast<FunctionTypePointer>(var->type()))
				{
					if (suffix != "selector" && suffix != "address")
					{
						m_errorReporter.typeError(9272_error, nativeLocationOf(_identifier), "Variables of type function pointer only support \".selector\" and \".address\".");
						return false;
					}
					if (fpType->kind() != FunctionType::Kind::External)
					{
						m_errorReporter.typeError(8533_error, nativeLocationOf(_identifier), "Only Variables of type external function pointer support \".selector\" and \".address\".");
						return false;
					}
				}
				else
				{
					m_errorReporter.typeError(3622_error, nativeLocationOf(_identifier), "The suffix \"." + suffix + "\" is not supported by this variable or type.");
					return false;
				}
			}
			else if (!var->isConstant() && var->isStateVariable())
			{
				m_errorReporter.typeError(
					1408_error,
					nativeLocationOf(_identifier),
					"Only local variables are supported. To access storage variables, use the \".slot\" and \".offset\" suffixes."
				);
				return false;
			}
			else if (var->type()->dataStoredIn(DataLocation::Storage))
			{
				m_errorReporter.typeError(9068_error, nativeLocationOf(_identifier), "You have to use the \".slot\" or \".offset\" suffix to access storage reference variables.");
				return false;
			}
			else if (var->type()->sizeOnStack() != 1)
			{
				if (
					auto const* arrayType = dynamic_cast<ArrayType const*>(var->type());
					arrayType && arrayType->isDynamicallySized() && arrayType->dataStoredIn(DataLocation::CallData)
				)
					m_errorReporter.typeError(1397_error, nativeLocationOf(_identifier), "Call data elements cannot be accessed directly. Use \".offset\" and \".length\" to access the calldata offset and length of this array and then use \"calldatacopy\".");
				else
				{
					solAssert(!var->type()->dataStoredIn(DataLocation::CallData), "");
					m_errorReporter.typeError(9857_error, nativeLocationOf(_identifier), "Only types that use one stack slot are supported.");
				}
				return false;
			}
		}
		else if (!identifierInfo.suffix.empty())
		{
			m_errorReporter.typeError(7944_error, nativeLocationOf(_identifier), "The suffixes \".offset\", \".slot\" and \".length\" can only be used with variables.");
			return false;
		}
		else if (_context == yul::IdentifierContext::LValue)
		{
			if (dynamic_cast<MagicVariableDeclaration const*>(declaration))
				return false;

			m_errorReporter.typeError(1990_error, nativeLocationOf(_identifier), "Only local variables can be assigned to in inline assembly.");
			return false;
		}

		if (_context == yul::IdentifierContext::RValue)
		{
			solAssert(!!declaration->type(), "Type of declaration required but not yet determined.");
			if (dynamic_cast<FunctionDefinition const*>(declaration))
			{
				m_errorReporter.declarationError(2025_error, nativeLocationOf(_identifier), "Access to functions is not allowed in inline assembly.");
				return false;
			}
			else if (dynamic_cast<VariableDeclaration const*>(declaration))
			{
			}
			else if (auto contract = dynamic_cast<ContractDefinition const*>(declaration))
			{
				if (!contract->isLibrary())
				{
					m_errorReporter.typeError(4977_error, nativeLocationOf(_identifier), "Expected a library.");
					return false;
				}
			}
			else
				return false;
		}
		identifierInfo.valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errorReporter,
		_inlineAssembly.dialect(),
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		solAssert(m_errorReporter.hasErrors());
	_inlineAssembly.annotation().hasMemoryEffects =
		lvalueAccessToMemoryVariable ||
		(analyzer.sideEffects().memory != yul::SideEffects::None);
	return false;
}

bool TypeChecker::visit(IfStatement const& _ifStatement)
{
	expectType(_ifStatement.condition(), *TypeProvider::boolean());
	_ifStatement.trueStatement().accept(*this);
	if (_ifStatement.falseStatement())
		_ifStatement.falseStatement()->accept(*this);
	return false;
}

void TypeChecker::endVisit(TryStatement const& _tryStatement)
{
	FunctionCall const* externalCall = dynamic_cast<FunctionCall const*>(&_tryStatement.externalCall());
	if (!externalCall || *externalCall->annotation().kind != FunctionCallKind::FunctionCall)
	{
		m_errorReporter.typeError(
			5347_error,
			_tryStatement.externalCall().location(),
			"Try can only be used with external function calls and contract creation calls."
		);
		return;
	}

	FunctionType const& functionType = dynamic_cast<FunctionType const&>(*externalCall->expression().annotation().type);
	if (
		functionType.kind() != FunctionType::Kind::External &&
		functionType.kind() != FunctionType::Kind::Creation &&
		functionType.kind() != FunctionType::Kind::DelegateCall
	)
	{
		m_errorReporter.typeError(
			2536_error,
			_tryStatement.externalCall().location(),
			"Try can only be used with external function calls and contract creation calls."
		);
		return;
	}

	externalCall->annotation().tryCall = true;

	solAssert(_tryStatement.clauses().size() >= 2, "");
	solAssert(_tryStatement.clauses().front(), "");

	TryCatchClause const& successClause = *_tryStatement.clauses().front();
	if (successClause.parameters())
	{
		TypePointers returnTypes =
			m_evmVersion.supportsReturndata() ?
			functionType.returnParameterTypes() :
			functionType.returnParameterTypesWithoutDynamicTypes();
		std::vector<ASTPointer<VariableDeclaration>> const& parameters =
			successClause.parameters()->parameters();
		if (returnTypes.size() != parameters.size())
			m_errorReporter.typeError(
				2800_error,
				successClause.location(),
				"Function returns " +
				to_string(functionType.returnParameterTypes().size()) +
				" values, but returns clause has " +
				to_string(parameters.size()) +
				" variables."
			);
		for (auto&& [parameter, returnType]: ranges::views::zip(parameters, returnTypes))
		{
			solAssert(returnType, "");
			if (parameter && *parameter->annotation().type != *returnType)
				m_errorReporter.typeError(
					6509_error,
					parameter->location(),
					"Invalid type, expected " +
					returnType->humanReadableName() +
					" but got " +
					parameter->annotation().type->humanReadableName() +
					"."
				);
		}
	}

	TryCatchClause const* panicClause = nullptr;
	TryCatchClause const* errorClause = nullptr;
	TryCatchClause const* lowLevelClause = nullptr;
	for (auto const& clause: _tryStatement.clauses() | ranges::views::drop_exactly(1) | views::dereferenceChecked)
	{
		if (clause.errorName() == "")
		{
			if (lowLevelClause)
				m_errorReporter.typeError(
					5320_error,
					clause.location(),
					SecondarySourceLocation{}.append("The first clause is here:", lowLevelClause->location()),
					"This try statement already has a low-level catch clause."
				);
			lowLevelClause = &clause;
			if (clause.parameters() && !clause.parameters()->parameters().empty())
			{
				if (
					clause.parameters()->parameters().size() != 1 ||
					*clause.parameters()->parameters().front()->type() != *TypeProvider::bytesMemory()
				)
					m_errorReporter.typeError(6231_error, clause.location(), "Expected `catch (bytes memory ...) { ... }` or `catch { ... }`.");
				if (!m_evmVersion.supportsReturndata())
					m_errorReporter.typeError(
						9908_error,
						clause.location(),
						"This catch clause type cannot be used on the selected EVM version (" +
						m_evmVersion.name() +
						"). You need at least a Byzantium-compatible EVM or use `catch { ... }`."
					);
			}
		}
		else if (clause.errorName() == "Error" || clause.errorName() == "Panic")
		{
			if (!m_evmVersion.supportsReturndata())
				m_errorReporter.typeError(
					1812_error,
					clause.location(),
					"This catch clause type cannot be used on the selected EVM version (" +
					m_evmVersion.name() +
					"). You need at least a Byzantium-compatible EVM or use `catch { ... }`."
				);

			if (clause.errorName() == "Error")
			{
				if (errorClause)
					m_errorReporter.typeError(
						1036_error,
						clause.location(),
						SecondarySourceLocation{}.append("The first clause is here:", errorClause->location()),
						"This try statement already has an \"Error\" catch clause."
					);
				errorClause = &clause;
				if (
					!clause.parameters() ||
					clause.parameters()->parameters().size() != 1 ||
					*clause.parameters()->parameters().front()->type() != *TypeProvider::stringMemory()
				)
					m_errorReporter.typeError(2943_error, clause.location(), "Expected `catch Error(string memory ...) { ... }`.");
			}
			else
			{
				if (panicClause)
					m_errorReporter.typeError(
						6732_error,
						clause.location(),
						SecondarySourceLocation{}.append("The first clause is here:", panicClause->location()),
						"This try statement already has a \"Panic\" catch clause."
					);
				panicClause = &clause;
				if (
					!clause.parameters() ||
					clause.parameters()->parameters().size() != 1 ||
					*clause.parameters()->parameters().front()->type() != *TypeProvider::uint256()
				)
					m_errorReporter.typeError(1271_error, clause.location(), "Expected `catch Panic(uint ...) { ... }`.");
			}
		}
		else
			m_errorReporter.typeError(
				3542_error,
				clause.location(),
				"Invalid catch clause name. Expected either `catch (...)`, `catch Error(...)`, or `catch Panic(...)`."
			);
	}
}

bool TypeChecker::visit(WhileStatement const& _whileStatement)
{
	expectType(_whileStatement.condition(), *TypeProvider::boolean());
	_whileStatement.body().accept(*this);
	return false;
}

bool TypeChecker::visit(ForStatement const& _forStatement)
{
	if (_forStatement.initializationExpression())
		_forStatement.initializationExpression()->accept(*this);
	if (_forStatement.condition())
		expectType(*_forStatement.condition(), *TypeProvider::boolean());
	if (_forStatement.loopExpression())
		_forStatement.loopExpression()->accept(*this);
	_forStatement.body().accept(*this);
	return false;
}

void TypeChecker::endVisit(Return const& _return)
{
	ParameterList const* params = _return.annotation().functionReturnParameters;
	if (!_return.expression())
	{
		if (params && !params->parameters().empty())
			m_errorReporter.typeError(6777_error, _return.location(), "Return arguments required.");
		return;
	}
	if (!params)
	{
		m_errorReporter.typeError(7552_error, _return.location(), "Return arguments not allowed.");
		return;
	}
	TypePointers returnTypes;
	for (auto const& var: params->parameters())
		returnTypes.push_back(type(*var));
	if (auto tupleType = dynamic_cast<TupleType const*>(type(*_return.expression())))
	{
		if (tupleType->components().size() != params->parameters().size())
			m_errorReporter.typeError(5132_error, _return.location(), "Different number of arguments in return statement than in returns declaration.");
		else
		{
			BoolResult result = tupleType->isImplicitlyConvertibleTo(TupleType(returnTypes));
			if (!result)
				m_errorReporter.typeErrorConcatenateDescriptions(
					5992_error,
					_return.expression()->location(),
					"Return argument type " +
					type(*_return.expression())->humanReadableName() +
					" is not implicitly convertible to expected type " +
					TupleType(returnTypes).humanReadableName() + ".",
					result.message()
				);
		}
	}
	else if (params->parameters().size() != 1)
		m_errorReporter.typeError(8863_error, _return.location(), "Different number of arguments in return statement than in returns declaration.");
	else
	{
		Type const* expected = type(*params->parameters().front());
		BoolResult result = type(*_return.expression())->isImplicitlyConvertibleTo(*expected);
		if (!result)
			m_errorReporter.typeErrorConcatenateDescriptions(
				6359_error,
				_return.expression()->location(),
				"Return argument type " +
				type(*_return.expression())->humanReadableName() +
				" is not implicitly convertible to expected type (type of first return variable) " +
				expected->humanReadableName() + ".",
				result.message()
			);
	}
}

void TypeChecker::endVisit(EmitStatement const& _emit)
{
	if (
		*_emit.eventCall().annotation().kind != FunctionCallKind::FunctionCall ||
		type(_emit.eventCall().expression())->category() != Type::Category::Function ||
		dynamic_cast<FunctionType const&>(*type(_emit.eventCall().expression())).kind() != FunctionType::Kind::Event
	)
		m_errorReporter.typeError(9292_error, _emit.eventCall().expression().location(), "Expression has to be an event invocation.");
}

void TypeChecker::endVisit(RevertStatement const& _revert)
{
	FunctionCall const& errorCall = _revert.errorCall();
	if (
		*errorCall.annotation().kind != FunctionCallKind::FunctionCall ||
		type(errorCall.expression())->category() != Type::Category::Function ||
		dynamic_cast<FunctionType const&>(*type(errorCall.expression())).kind() != FunctionType::Kind::Error
	)
		m_errorReporter.typeError(1885_error, errorCall.expression().location(), "Expression has to be an error.");
}

void TypeChecker::endVisit(ArrayTypeName const& _typeName)
{
	solAssert(
		_typeName.baseType().annotation().type &&
		_typeName.baseType().annotation().type->storageBytes() != 0,
		"Illegal base type of storage size zero for array."
	);
}

bool TypeChecker::visit(VariableDeclarationStatement const& _statement)
{
	if (!_statement.initialValue())
	{
		// No initial value is only permitted for single variables with specified type.
		// This usually already results in a parser error.
		if (_statement.declarations().size() != 1 || !_statement.declarations().front())
		{
			solAssert(m_errorReporter.hasErrors(), "");

			// It is okay to return here, as there are no named components on the
			// left-hand-side that could cause any damage later.
			return false;
		}

		VariableDeclaration const& varDecl = *_statement.declarations().front();
		solAssert(varDecl.annotation().type, "");

		if (dynamic_cast<MappingType const*>(type(varDecl)))
			m_errorReporter.typeError(
				4182_error,
				varDecl.location(),
				"Uninitialized mapping. Mappings cannot be created dynamically, you have to assign them from a state variable."
			);
		varDecl.accept(*this);
		return false;
	}

	// Here we have an initial value and might have to derive some types before we can visit
	// the variable declaration(s).

	_statement.initialValue()->accept(*this);
	TypePointers valueTypes;
	if (auto tupleType = dynamic_cast<TupleType const*>(type(*_statement.initialValue())))
		valueTypes = tupleType->components();
	else
		valueTypes = TypePointers{type(*_statement.initialValue())};

	vector<ASTPointer<VariableDeclaration>> const& variables = _statement.declarations();
	if (variables.empty())
		// We already have an error for this in the SyntaxChecker.
		solAssert(m_errorReporter.hasErrors(), "");
	else if (valueTypes.size() != variables.size())
		m_errorReporter.typeError(
			7364_error,
			_statement.location(),
			"Different number of components on the left hand side (" +
			toString(variables.size()) +
			") than on the right hand side (" +
			toString(valueTypes.size()) +
			")."
		);

	for (size_t i = 0; i < min(variables.size(), valueTypes.size()); ++i)
	{
		if (!variables[i])
			continue;
		VariableDeclaration const& var = *variables[i];
		solAssert(!var.value(), "Value has to be tied to statement.");
		Type const* valueComponentType = valueTypes[i];
		solAssert(!!valueComponentType, "");
		solAssert(var.annotation().type, "");

		var.accept(*this);
		BoolResult result = valueComponentType->isImplicitlyConvertibleTo(*var.annotation().type);
		if (!result)
		{
			auto errorMsg = "Type " +
				valueComponentType->humanReadableName() +
				" is not implicitly convertible to expected type " +
				var.annotation().type->humanReadableName();
			if (
				valueComponentType->category() == Type::Category::RationalNumber &&
				dynamic_cast<RationalNumberType const&>(*valueComponentType).isFractional() &&
				valueComponentType->mobileType()
			)
			{
				if (var.annotation().type->operator==(*valueComponentType->mobileType()))
					m_errorReporter.typeError(
						5107_error,
						_statement.location(),
						errorMsg + ", but it can be explicitly converted."
					);
				else
					m_errorReporter.typeError(
						4486_error,
						_statement.location(),
						errorMsg +
						". Try converting to type " +
						valueComponentType->mobileType()->humanReadableName() +
						" or use an explicit conversion."
					);
			}
			else
				m_errorReporter.typeErrorConcatenateDescriptions(
					9574_error,
					_statement.location(),
					errorMsg + ".",
					result.message()
				);
		}
	}

	if (valueTypes.size() != variables.size())
	{
		solAssert(m_errorReporter.hasErrors(), "Should have errors!");
		for (auto const& var: variables)
			if (var && !var->annotation().type)
				BOOST_THROW_EXCEPTION(FatalError());
	}

	return false;
}

void TypeChecker::endVisit(ExpressionStatement const& _statement)
{
	if (type(_statement.expression())->category() == Type::Category::RationalNumber)
		if (!dynamic_cast<RationalNumberType const&>(*type(_statement.expression())).mobileType())
			m_errorReporter.typeError(3757_error, _statement.expression().location(), "Invalid rational number.");

	if (auto call = dynamic_cast<FunctionCall const*>(&_statement.expression()))
	{
		if (auto callType = dynamic_cast<FunctionType const*>(type(call->expression())))
		{
			auto kind = callType->kind();
			if (
				kind == FunctionType::Kind::BareCall ||
				kind == FunctionType::Kind::BareCallCode ||
				kind == FunctionType::Kind::BareDelegateCall ||
				kind == FunctionType::Kind::BareStaticCall
			)
				m_errorReporter.warning(9302_error, _statement.location(), "Return value of low-level calls not used.");
			else if (kind == FunctionType::Kind::Send)
				m_errorReporter.warning(5878_error, _statement.location(), "Failure condition of 'send' ignored. Consider using 'transfer' instead.");
		}
	}
}

bool TypeChecker::visit(Conditional const& _conditional)
{
	expectType(_conditional.condition(), *TypeProvider::boolean());

	_conditional.trueExpression().accept(*this);
	_conditional.falseExpression().accept(*this);

	Type const* trueType = type(_conditional.trueExpression())->mobileType();
	Type const* falseType = type(_conditional.falseExpression())->mobileType();

	Type const* commonType = nullptr;

	if (!trueType)
		m_errorReporter.typeError(9717_error, _conditional.trueExpression().location(), "Invalid mobile type in true expression.");
	else
		commonType = trueType;

	if (!falseType)
		m_errorReporter.typeError(3703_error, _conditional.falseExpression().location(), "Invalid mobile type in false expression.");
	else
		commonType = falseType;

	if (!trueType && !falseType)
		BOOST_THROW_EXCEPTION(FatalError());
	else if (trueType && falseType)
	{
		commonType = Type::commonType(trueType, falseType);

		if (!commonType)
		{
			m_errorReporter.typeError(
					1080_error,
					_conditional.location(),
					"True expression's type " +
					trueType->humanReadableName() +
					" does not match false expression's type " +
					falseType->humanReadableName() +
					"."
					);
			// even we can't find a common type, we have to set a type here,
			// otherwise the upper statement will not be able to check the type.
			commonType = trueType;
		}
	}

	_conditional.annotation().isConstant = false;
	_conditional.annotation().type = commonType;
	_conditional.annotation().isPure =
		*_conditional.condition().annotation().isPure &&
		*_conditional.trueExpression().annotation().isPure &&
		*_conditional.falseExpression().annotation().isPure;

	_conditional.annotation().isLValue = false;

	if (_conditional.annotation().willBeWrittenTo)
		m_errorReporter.typeError(
			2212_error,
			_conditional.location(),
			"Conditional expression as left value is not supported yet."
		);

	return false;
}

void TypeChecker::checkExpressionAssignment(Type const& _type, Expression const& _expression)
{
	if (auto const* tupleExpression = dynamic_cast<TupleExpression const*>(&_expression))
	{
		if (tupleExpression->components().empty())
			m_errorReporter.typeError(5547_error, _expression.location(), "Empty tuple on the left hand side.");

		auto const* tupleType = dynamic_cast<TupleType const*>(&_type);
		auto const& types = tupleType && tupleExpression->components().size() != 1 ? tupleType->components() : vector<Type const*> { &_type };

		solAssert(
			tupleExpression->components().size() == types.size() || m_errorReporter.hasErrors(),
			"Array sizes don't match and no errors generated."
		);

		for (size_t i = 0; i < min(tupleExpression->components().size(), types.size()); i++)
			if (types[i])
			{
				solAssert(!!tupleExpression->components()[i], "");
				checkExpressionAssignment(*types[i], *tupleExpression->components()[i]);
			}
	}
	else if (_type.nameable() && _type.containsNestedMapping())
	{
		bool isLocalOrReturn = false;
		if (auto const* identifier = dynamic_cast<Identifier const*>(&_expression))
			if (auto const *variableDeclaration = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
				if (variableDeclaration->isLocalOrReturn())
					isLocalOrReturn = true;
		if (!isLocalOrReturn)
			m_errorReporter.typeError(9214_error, _expression.location(), "Types in storage containing (nested) mappings cannot be assigned to.");
	}
}

bool TypeChecker::visit(Assignment const& _assignment)
{
	requireLValue(
		_assignment.leftHandSide(),
		_assignment.assignmentOperator() == Token::Assign
	);
	Type const* t = type(_assignment.leftHandSide());
	_assignment.annotation().type = t;
	_assignment.annotation().isPure = false;
	_assignment.annotation().isLValue = false;
	_assignment.annotation().isConstant = false;

	checkExpressionAssignment(*t, _assignment.leftHandSide());

	if (TupleType const* tupleType = dynamic_cast<TupleType const*>(t))
	{
		if (_assignment.assignmentOperator() != Token::Assign)
			m_errorReporter.typeError(
				4289_error,
				_assignment.location(),
				"Compound assignment is not allowed for tuple types."
			);
		// Sequenced assignments of tuples is not valid, make the result a "void" type.
		_assignment.annotation().type = TypeProvider::emptyTuple();

		expectType(_assignment.rightHandSide(), *tupleType);

		// expectType does not cause fatal errors, so we have to check again here.
		if (dynamic_cast<TupleType const*>(type(_assignment.rightHandSide())))
			checkDoubleStorageAssignment(_assignment);
	}
	else if (_assignment.assignmentOperator() == Token::Assign)
		expectType(_assignment.rightHandSide(), *t);
	else
	{
		// compound assignment
		_assignment.rightHandSide().accept(*this);
		Type const* resultType = t->binaryOperatorResult(
			TokenTraits::AssignmentToBinaryOp(_assignment.assignmentOperator()),
			type(_assignment.rightHandSide())
		);
		if (!resultType || *resultType != *t)
			m_errorReporter.typeError(
				7366_error,
				_assignment.location(),
				"Operator " +
				string(TokenTraits::toString(_assignment.assignmentOperator())) +
				" not compatible with types " +
				t->humanReadableName() +
				" and " +
				type(_assignment.rightHandSide())->humanReadableName() +
				"."
			);
	}
	return false;
}

bool TypeChecker::visit(TupleExpression const& _tuple)
{
	_tuple.annotation().isConstant = false;
	vector<ASTPointer<Expression>> const& components = _tuple.components();
	TypePointers types;

	if (_tuple.annotation().willBeWrittenTo)
	{
		if (_tuple.isInlineArray())
			m_errorReporter.fatalTypeError(3025_error, _tuple.location(), "Inline array type cannot be declared as LValue.");
		for (auto const& component: components)
			if (component)
			{
				requireLValue(
					*component,
					_tuple.annotation().lValueOfOrdinaryAssignment
				);
				types.push_back(type(*component));
			}
			else
				types.push_back(nullptr);
		if (components.size() == 1)
			_tuple.annotation().type = type(*components[0]);
		else
			_tuple.annotation().type = TypeProvider::tuple(std::move(types));
		// If some of the components are not LValues, the error is reported above.
		_tuple.annotation().isLValue = true;
		_tuple.annotation().isPure = false;
	}
	else
	{
		bool isPure = true;
		Type const* inlineArrayType = nullptr;

		for (size_t i = 0; i < components.size(); ++i)
		{
			if (!components[i])
				m_errorReporter.fatalTypeError(8381_error, _tuple.location(), "Tuple component cannot be empty.");

			components[i]->accept(*this);
			types.push_back(type(*components[i]));

			if (types[i]->category() == Type::Category::Tuple)
				if (dynamic_cast<TupleType const&>(*types[i]).components().empty())
				{
					if (_tuple.isInlineArray())
						m_errorReporter.fatalTypeError(5604_error, components[i]->location(), "Array component cannot be empty.");
					m_errorReporter.typeError(6473_error, components[i]->location(), "Tuple component cannot be empty.");
				}

			// Note: code generation will visit each of the expression even if they are not assigned from.
			if (types[i]->category() == Type::Category::RationalNumber && components.size() > 1)
				if (!dynamic_cast<RationalNumberType const&>(*types[i]).mobileType())
					m_errorReporter.fatalTypeError(3390_error, components[i]->location(), "Invalid rational number.");

			if (_tuple.isInlineArray())
			{
				solAssert(!!types[i], "Inline array cannot have empty components");

				if ((i == 0 || inlineArrayType) && !types[i]->mobileType())
					m_errorReporter.fatalTypeError(9563_error, components[i]->location(), "Invalid mobile type.");

				if (i == 0)
					inlineArrayType = types[i]->mobileType();
				else if (inlineArrayType)
					inlineArrayType = Type::commonType(inlineArrayType, types[i]);
			}
			if (!*components[i]->annotation().isPure)
				isPure = false;
		}
		_tuple.annotation().isPure = isPure;
		if (_tuple.isInlineArray())
		{
			if (!inlineArrayType)
				m_errorReporter.fatalTypeError(6378_error, _tuple.location(), "Unable to deduce common type for array elements.");
			else if (!inlineArrayType->nameable())
				m_errorReporter.fatalTypeError(
					9656_error,
					_tuple.location(),
					"Unable to deduce nameable type for array elements. Try adding explicit type conversion for the first element."
				);
			else if (inlineArrayType->containsNestedMapping())
				m_errorReporter.fatalTypeError(
					1545_error,
					_tuple.location(),
					"Type " + inlineArrayType->humanReadableName() + " is only valid in storage."
				);

			_tuple.annotation().type = TypeProvider::array(DataLocation::Memory, inlineArrayType, types.size());
		}
		else
		{
			if (components.size() == 1)
				_tuple.annotation().type = type(*components[0]);
			else
				_tuple.annotation().type = TypeProvider::tuple(std::move(types));
		}

		_tuple.annotation().isLValue = false;
	}
	return false;
}

bool TypeChecker::visit(UnaryOperation const& _operation)
{
	// Inc, Dec, Add, Sub, Not, BitNot, Delete
	Token op = _operation.getOperator();
	bool const modifying = (op == Token::Inc || op == Token::Dec || op == Token::Delete);
	if (modifying)
		requireLValue(_operation.subExpression(), false);
	else
		_operation.subExpression().accept(*this);
	Type const* subExprType = type(_operation.subExpression());


	// Check if the operator is built-in or user-defined.
	FunctionDefinition const* userDefinedOperator = subExprType->userDefinedOperator(
		_operation.getOperator(),
		*currentDefinitionScope()
	);
	_operation.annotation().userDefinedFunction = userDefinedOperator;
	FunctionType const* userDefinedFunctionType = nullptr;
	if (userDefinedOperator)
		userDefinedFunctionType = &dynamic_cast<FunctionType const&>(
			userDefinedOperator->libraryFunction() ?
			*userDefinedOperator->typeViaContractName() :
			*userDefinedOperator->type()
		);

	TypeResult builtinResult = subExprType->unaryOperatorResult(op);

	solAssert(!builtinResult || !userDefinedOperator);
	if (userDefinedOperator)
	{
		solAssert(userDefinedFunctionType->parameterTypes().size() == 1);
		solAssert(userDefinedFunctionType->returnParameterTypes().size() == 1);
		solAssert(
			*userDefinedFunctionType->parameterTypes().at(0) ==
			*userDefinedFunctionType->returnParameterTypes().at(0)
		);
		_operation.annotation().type = userDefinedFunctionType->returnParameterTypes().at(0);
	}
	else if (builtinResult)
		_operation.annotation().type = builtinResult;
	else
	{
		string description = "Unary operator " + string(TokenTraits::toString(op)) + " cannot be applied to type " + subExprType->humanReadableName() + "." + (!builtinResult.message().empty() ? " " + builtinResult.message() : "");
		if (modifying)
			// Cannot just report the error, ignore the unary operator, and continue,
			// because the sub-expression was already processed with requireLValue()
			m_errorReporter.fatalTypeError(9767_error, _operation.location(), description);
		else
			m_errorReporter.typeError(4907_error, _operation.location(), description);
		_operation.annotation().type = subExprType;
	}

	_operation.annotation().isConstant = false;
	_operation.annotation().isPure =
		!modifying &&
		*_operation.subExpression().annotation().isPure &&
		(!userDefinedFunctionType || userDefinedFunctionType->isPure());
	_operation.annotation().isLValue = false;

	return false;
}

void TypeChecker::endVisit(BinaryOperation const& _operation)
{
	Type const* leftType = type(_operation.leftExpression());
	Type const* rightType = type(_operation.rightExpression());
	_operation.annotation().isLValue = false;
	_operation.annotation().isConstant = false;

	// Check if the operator is built-in or user-defined.
	FunctionDefinition const* userDefinedOperator = leftType->userDefinedOperator(
		_operation.getOperator(),
		*currentDefinitionScope()
	);
	_operation.annotation().userDefinedFunction = userDefinedOperator;
	FunctionType const* userDefinedFunctionType = nullptr;
	if (userDefinedOperator)
		userDefinedFunctionType = &dynamic_cast<FunctionType const&>(
			userDefinedOperator->libraryFunction() ?
			*userDefinedOperator->typeViaContractName() :
			*userDefinedOperator->type()
		);
	_operation.annotation().isPure =
		*_operation.leftExpression().annotation().isPure &&
		*_operation.rightExpression().annotation().isPure &&
		(!userDefinedFunctionType || userDefinedFunctionType->isPure());

	TypeResult builtinResult = leftType->binaryOperatorResult(_operation.getOperator(), rightType);
	Type const* commonType = leftType;

	// Either the operator is user-defined or built-in.
	// TODO For enums, we have compare operators. Should we disallow overriding them?
	solAssert(!userDefinedOperator || !builtinResult);

	if (!builtinResult && !userDefinedOperator)
		m_errorReporter.typeError(
			2271_error,
			_operation.location(),
			"Operator " +
			string(TokenTraits::toString(_operation.getOperator())) +
			" not compatible with types " +
			leftType->humanReadableName() +
			" and " +
			rightType->humanReadableName() + "." +
			(!builtinResult.message().empty() ? " " + builtinResult.message() : "")
		);

	if (builtinResult)
		commonType = builtinResult.get();
	else if (userDefinedOperator)
	{
		solAssert(
			userDefinedFunctionType->parameterTypes().size() == 2 &&
			*userDefinedFunctionType->parameterTypes().at(0) ==
			*userDefinedFunctionType->parameterTypes().at(1)
		);
		commonType = userDefinedFunctionType->returnParameterTypes().at(0);
	}

	_operation.annotation().commonType = commonType;
	_operation.annotation().type =
		TokenTraits::isCompareOp(_operation.getOperator()) ?
		TypeProvider::boolean() :
		commonType;

	if (userDefinedOperator)
		solAssert(
			userDefinedFunctionType->returnParameterTypes().size() == 1 &&
			*userDefinedFunctionType->returnParameterTypes().front() == *_operation.annotation().type
		);
	else if (builtinResult && (_operation.getOperator() == Token::Exp || _operation.getOperator() == Token::SHL))
	{
		string operation = _operation.getOperator() == Token::Exp ? "exponentiation" : "shift";
		if (
			leftType->category() == Type::Category::RationalNumber &&
			rightType->category() != Type::Category::RationalNumber
		)
		{
			// These rules are enforced by the binary operator, but assert them here too.
			if (auto type = dynamic_cast<IntegerType const*>(commonType))
				solAssert(type->numBits() == 256, "");
			if (auto type = dynamic_cast<FixedPointType const*>(commonType))
				solAssert(type->numBits() == 256, "");
		}
		if (
			commonType->category() == Type::Category::Integer &&
			rightType->category() == Type::Category::Integer &&
			dynamic_cast<IntegerType const&>(*commonType).numBits() <
			dynamic_cast<IntegerType const&>(*rightType).numBits()
		)
			m_errorReporter.warning(
				3149_error,
				_operation.location(),
				"The result type of the " +
				operation +
				" operation is equal to the type of the first operand (" +
				commonType->humanReadableName() +
				") ignoring the (larger) type of the second operand (" +
				rightType->humanReadableName() +
				") which might be unexpected. Silence this warning by either converting "
				"the first or the second operand to the type of the other."
			);
	}
}

Type const* TypeChecker::typeCheckTypeConversionAndRetrieveReturnType(
	FunctionCall const& _functionCall
)
{
	solAssert(*_functionCall.annotation().kind == FunctionCallKind::TypeConversion, "");
	Type const* expressionType = type(_functionCall.expression());

	vector<ASTPointer<Expression const>> const& arguments = _functionCall.arguments();
	bool const isPositionalCall = _functionCall.names().empty();

	Type const* resultType = dynamic_cast<TypeType const&>(*expressionType).actualType();
	if (arguments.size() != 1)
		m_errorReporter.typeError(
			2558_error,
			_functionCall.location(),
			"Exactly one argument expected for explicit type conversion."
		);
	else if (!isPositionalCall)
		m_errorReporter.typeError(
			5153_error,
			_functionCall.location(),
			"Type conversion cannot allow named arguments."
		);
	else
	{
		Type const* argType = type(*arguments.front());
		// Resulting data location is memory unless we are converting from a reference
		// type with a different data location.
		// (data location cannot yet be specified for type conversions)
		DataLocation dataLoc = DataLocation::Memory;
		if (auto argRefType = dynamic_cast<ReferenceType const*>(argType))
			dataLoc = argRefType->location();
		if (auto type = dynamic_cast<ReferenceType const*>(resultType))
			resultType = TypeProvider::withLocation(type, dataLoc, type->isPointer());
		BoolResult result = argType->isExplicitlyConvertibleTo(*resultType);
		if (result)
		{
			if (auto argArrayType = dynamic_cast<ArrayType const*>(argType))
			{
				if (auto resultArrayType = dynamic_cast<ArrayType const*>(resultType))
					solAssert(
						argArrayType->location() != DataLocation::Storage ||
						(
							(
								resultArrayType->isPointer() ||
								(argArrayType->isByteArrayOrString() && resultArrayType->isByteArrayOrString())
							) &&
							resultArrayType->location() == DataLocation::Storage
						),
						"Invalid explicit conversion to storage type."
					);
				else
					solAssert(
						argArrayType->isByteArray() && resultType->category() == Type::Category::FixedBytes,
						""
					);
			}
		}
		else
		{
			if (
				resultType->category() == Type::Category::Contract &&
				argType->category() == Type::Category::Address
			)
			{
				solAssert(dynamic_cast<ContractType const*>(resultType)->isPayable(), "");
				solAssert(
					dynamic_cast<AddressType const*>(argType)->stateMutability() <
						StateMutability::Payable,
					""
				);
				SecondarySourceLocation ssl;
				if (
					auto const* identifier = dynamic_cast<Identifier const*>(arguments.front().get())
				)
					if (
						auto const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(
							identifier->annotation().referencedDeclaration
						)
					)
						ssl.append(
							"Did you mean to declare this variable as \"address payable\"?",
							variableDeclaration->location()
						);
				m_errorReporter.typeError(
					7398_error,
					_functionCall.location(),
					ssl,
					"Explicit type conversion not allowed from non-payable \"address\" to \"" +
					resultType->humanReadableName() +
					"\", which has a payable fallback function."
				);
			}
			else if (
				auto const* functionType = dynamic_cast<FunctionType const*>(argType);
				functionType &&
				functionType->kind() == FunctionType::Kind::External &&
				resultType->category() == Type::Category::Address
			)
				m_errorReporter.typeError(
					5030_error,
					_functionCall.location(),
					"Explicit type conversion not allowed from \"" +
					argType->humanReadableName() +
					"\" to \"" +
					resultType->humanReadableName() +
					"\". To obtain the address of the contract of the function, " +
					"you can use the .address member of the function."
				);
			else
				m_errorReporter.typeErrorConcatenateDescriptions(
					9640_error,
					_functionCall.location(),
					"Explicit type conversion not allowed from \"" +
					argType->humanReadableName() +
					"\" to \"" +
					resultType->humanReadableName() +
					"\".",
					result.message()
				);
		}
	}
	return resultType;
}

void TypeChecker::typeCheckFunctionCall(
	FunctionCall const& _functionCall,
	FunctionTypePointer _functionType
)
{
	// Actual function call or struct constructor call.

	solAssert(!!_functionType, "");
	solAssert(_functionType->kind() != FunctionType::Kind::ABIDecode, "");

	if (_functionType->kind() == FunctionType::Kind::Declaration)
	{
		solAssert(_functionType->declaration().annotation().contract, "");
		if (
			m_currentContract &&
			m_currentContract->derivesFrom(*_functionType->declaration().annotation().contract) &&
			!dynamic_cast<FunctionDefinition const&>(_functionType->declaration()).isImplemented()
		)
			m_errorReporter.typeError(
				7501_error,
				_functionCall.location(),
				"Cannot call unimplemented base function."
			);
		else
			m_errorReporter.typeError(
				3419_error,
				_functionCall.location(),
				"Cannot call function via contract type name."
			);
		return;
	}

	// Check for unsupported use of bare static call
	if (
		_functionType->kind() == FunctionType::Kind::BareStaticCall &&
		!m_evmVersion.hasStaticCall()
	)
		m_errorReporter.typeError(
			5052_error,
			_functionCall.location(),
			"\"staticcall\" is not supported by the VM version."
		);

	// Perform standard function call type checking
	typeCheckFunctionGeneralChecks(_functionCall, _functionType);
}

void TypeChecker::typeCheckFallbackFunction(FunctionDefinition const& _function)
{
	solAssert(_function.isFallback(), "");

	if (_function.libraryFunction())
		m_errorReporter.typeError(5982_error, _function.location(), "Libraries cannot have fallback functions.");
	if (_function.stateMutability() != StateMutability::NonPayable && _function.stateMutability() != StateMutability::Payable)
		m_errorReporter.typeError(
			4575_error,
			_function.location(),
			"Fallback function must be payable or non-payable, but is \"" +
			stateMutabilityToString(_function.stateMutability()) +
			"\"."
		);
	if (_function.visibility() != Visibility::External)
		m_errorReporter.typeError(1159_error, _function.location(), "Fallback function must be defined as \"external\".");

	if (!_function.returnParameters().empty() || !_function.parameters().empty())
	{
		if (
			_function.returnParameters().size() != 1 ||
			*type(*_function.returnParameters().front()) != *TypeProvider::bytesMemory() ||
			_function.parameters().size() != 1 ||
			*type(*_function.parameters().front()) != *TypeProvider::bytesCalldata()
		)
			m_errorReporter.typeError(
				5570_error,
				_function.returnParameterList()->location(),
				"Fallback function either has to have the signature \"fallback()\" or \"fallback(bytes calldata) returns (bytes memory)\"."
			);
	}
}

void TypeChecker::typeCheckConstructor(FunctionDefinition const& _function)
{
	solAssert(_function.isConstructor(), "");
	if (_function.overrides())
		m_errorReporter.typeError(1209_error, _function.location(), "Constructors cannot override.");
	if (!_function.returnParameters().empty())
		m_errorReporter.typeError(9712_error, _function.returnParameterList()->location(), "Non-empty \"returns\" directive for constructor.");
	if (_function.stateMutability() != StateMutability::NonPayable && _function.stateMutability() != StateMutability::Payable)
		m_errorReporter.typeError(
			1558_error,
			_function.location(),
			"Constructor must be payable or non-payable, but is \"" +
			stateMutabilityToString(_function.stateMutability()) +
			"\"."
		);
	if (!_function.noVisibilitySpecified())
	{
		auto const& contract = dynamic_cast<ContractDefinition const&>(*_function.scope());
		if (_function.visibility() != Visibility::Public && _function.visibility() != Visibility::Internal)
			m_errorReporter.typeError(9239_error, _function.location(), "Constructor cannot have visibility.");
		else if (_function.isPublic() && contract.abstract())
			m_errorReporter.declarationError(
				8295_error,
				_function.location(),
				"Abstract contracts cannot have public constructors. Remove the \"public\" keyword to fix this."
			);
		else if (!_function.isPublic() && !contract.abstract())
			m_errorReporter.declarationError(
				1845_error,
				_function.location(),
				"Non-abstract contracts cannot have internal constructors. Remove the \"internal\" keyword and make the contract abstract to fix this."
			);
		else
			m_errorReporter.warning(
				2462_error,
				_function.location(),
				"Visibility for constructor is ignored. If you want the contract to be non-deployable, making it \"abstract\" is sufficient."
			);
	}
}

void TypeChecker::typeCheckABIEncodeFunctions(
	FunctionCall const& _functionCall,
	FunctionTypePointer _functionType
)
{
	solAssert(!!_functionType, "");
	solAssert(
		_functionType->kind() == FunctionType::Kind::ABIEncode ||
		_functionType->kind() == FunctionType::Kind::ABIEncodePacked ||
		_functionType->kind() == FunctionType::Kind::ABIEncodeWithSelector ||
		_functionType->kind() == FunctionType::Kind::ABIEncodeCall ||
		_functionType->kind() == FunctionType::Kind::ABIEncodeWithSignature,
		"ABI function has unexpected FunctionType::Kind."
	);
	solAssert(_functionType->takesArbitraryParameters(), "ABI functions should be variadic.");

	bool const isPacked = _functionType->kind() == FunctionType::Kind::ABIEncodePacked;
	solAssert(_functionType->padArguments() != isPacked, "ABI function with unexpected padding");

	bool const abiEncoderV2 = useABICoderV2();

	// Check for named arguments
	if (!_functionCall.names().empty())
	{
		m_errorReporter.typeError(
			2627_error,
			_functionCall.location(),
			"Named arguments cannot be used for functions that take arbitrary parameters."
		);
		return;
	}

	// Perform standard function call type checking
	typeCheckFunctionGeneralChecks(_functionCall, _functionType);

	// No further generic checks needed as we do a precise check for ABIEncodeCall
	if (_functionType->kind() == FunctionType::Kind::ABIEncodeCall)
	{
		typeCheckABIEncodeCallFunction(_functionCall);
		return;
	}

	// Check additional arguments for variadic functions
	vector<ASTPointer<Expression const>> const& arguments = _functionCall.arguments();
	for (size_t i = 0; i < arguments.size(); ++i)
	{
		auto const& argType = type(*arguments[i]);

		if (argType->category() == Type::Category::RationalNumber)
		{
			auto const& rationalType = dynamic_cast<RationalNumberType const&>(*argType);
			if (rationalType.isFractional())
			{
				m_errorReporter.typeError(
					6090_error,
					arguments[i]->location(),
					"Fractional numbers cannot yet be encoded."
				);
				continue;
			}
			else if (!argType->mobileType())
			{
				m_errorReporter.typeError(
					8009_error,
					arguments[i]->location(),
					"Invalid rational number (too large or division by zero)."
				);
				continue;
			}
			else if (isPacked)
			{
				m_errorReporter.typeError(
					7279_error,
					arguments[i]->location(),
					"Cannot perform packed encoding for a literal."
					" Please convert it to an explicit type first."
				);
				continue;
			}
		}

		if (isPacked && !typeSupportedByOldABIEncoder(*argType, false /* isLibrary */))
		{
			m_errorReporter.typeError(
				9578_error,
				arguments[i]->location(),
				"Type not supported in packed mode."
			);
			continue;
		}

		if (!argType->fullEncodingType(false, abiEncoderV2, !_functionType->padArguments()))
			m_errorReporter.typeError(
				2056_error,
				arguments[i]->location(),
				"This type cannot be encoded."
			);
	}
}

void TypeChecker::typeCheckABIEncodeCallFunction(FunctionCall const& _functionCall)
{
	vector<ASTPointer<Expression const>> const& arguments = _functionCall.arguments();

	// Expecting first argument to be the function pointer and second to be a tuple.
	if (arguments.size() != 2)
	{
		m_errorReporter.typeError(
			6219_error,
			_functionCall.location(),
			"Expected two arguments: a function pointer followed by a tuple."
		);
		return;
	}

	FunctionType const* externalFunctionType = nullptr;
	if (auto const functionPointerType = dynamic_cast<FunctionTypePointer>(type(*arguments.front())))
	{
		// this cannot be a library function, that is checked below
		externalFunctionType = functionPointerType->asExternallyCallableFunction(false);
		solAssert(externalFunctionType->kind() == functionPointerType->kind());
	}
	else
	{
		m_errorReporter.typeError(
			5511_error,
			arguments.front()->location(),
			"Expected first argument to be a function pointer, not \"" +
			type(*arguments.front())->humanReadableName() +
			"\"."
		);
		return;
	}

	if (
		externalFunctionType->kind() != FunctionType::Kind::External &&
		externalFunctionType->kind() != FunctionType::Kind::Declaration
	)
	{
		string msg = "Expected regular external function type, or external view on public function.";

		switch (externalFunctionType->kind())
		{
			case FunctionType::Kind::Internal:
				msg += " Provided internal function.";
				break;
			case FunctionType::Kind::DelegateCall:
				msg += " Cannot use library functions for abi.encodeCall.";
				break;
			case FunctionType::Kind::Creation:
				msg += " Provided creation function.";
				break;
			case FunctionType::Kind::Event:
				msg += " Cannot use events for abi.encodeCall.";
				break;
			case FunctionType::Kind::Error:
				msg += " Cannot use errors for abi.encodeCall.";
				break;
			default:
				msg += " Cannot use special function.";
		}

		SecondarySourceLocation ssl{};

		if (externalFunctionType->hasDeclaration())
		{
			ssl.append("Function is declared here:", externalFunctionType->declaration().location());
			if (
				externalFunctionType->declaration().visibility() == Visibility::Public &&
				externalFunctionType->declaration().scope() == m_currentContract
			)
				msg += " Did you forget to prefix \"this.\"?";
			else if (
				m_currentContract &&
				externalFunctionType->declaration().scope() != m_currentContract &&
				util::contains(
					m_currentContract->annotation().linearizedBaseContracts,
					externalFunctionType->declaration().scope()
				)
			)
				msg += " Functions from base contracts have to be external.";
		}

		m_errorReporter.typeError(3509_error, arguments[0]->location(), ssl, msg);
		return;
	}
	solAssert(!externalFunctionType->takesArbitraryParameters(), "Function must have fixed parameters.");
	// Tuples with only one component become that component
	vector<ASTPointer<Expression const>> callArguments;

	auto const* tupleType = dynamic_cast<TupleType const*>(type(*arguments[1]));
	if (tupleType)
	{
		if (TupleExpression const* argumentTuple = dynamic_cast<TupleExpression const*>(arguments[1].get()))
			callArguments = decltype(callArguments){argumentTuple->components().begin(), argumentTuple->components().end()};
		else
		{
			m_errorReporter.typeError(
				9062_error,
				arguments[1]->location(),
				"Expected an inline tuple, not an expression of a tuple type."
			);
			return;
		}
	}
	else
		callArguments.push_back(arguments[1]);

	if (externalFunctionType->parameterTypes().size() != callArguments.size())
	{
		if (tupleType)
			m_errorReporter.typeError(
				7788_error,
				_functionCall.location(),
				"Expected " +
				to_string(externalFunctionType->parameterTypes().size()) +
				" instead of " +
				to_string(callArguments.size()) +
				" components for the tuple parameter."
			);
		else
			m_errorReporter.typeError(
				7515_error,
				_functionCall.location(),
				"Expected a tuple with " +
				to_string(externalFunctionType->parameterTypes().size()) +
				" components instead of a single non-tuple parameter."
			);
	}

	// Use min() to check as much as we can before failing fatally
	size_t const numParameters = min(callArguments.size(), externalFunctionType->parameterTypes().size());

	for (size_t i = 0; i < numParameters; i++)
	{
		Type const& argType = *type(*callArguments[i]);
		BoolResult result = argType.isImplicitlyConvertibleTo(*externalFunctionType->parameterTypes()[i]);
		if (!result)
			m_errorReporter.typeError(
				5407_error,
				callArguments[i]->location(),
				"Cannot implicitly convert component at position " +
				to_string(i) +
				" from \"" +
				argType.humanReadableName() +
				"\" to \"" +
				externalFunctionType->parameterTypes()[i]->humanReadableName() +
				"\"" +
				(result.message().empty() ?  "." : ": " + result.message())
			);
	}
}


void TypeChecker::typeCheckStringConcatFunction(
	FunctionCall const& _functionCall,
	FunctionType const* _functionType
)
{
	solAssert(_functionType);
	solAssert(_functionType->kind() == FunctionType::Kind::StringConcat);
	solAssert(_functionCall.names().empty());

	typeCheckFunctionGeneralChecks(_functionCall, _functionType);

	for (shared_ptr<Expression const> const& argument: _functionCall.arguments())
	{
		Type const* argumentType = type(*argument);
		bool notConvertibleToString = !argumentType->isImplicitlyConvertibleTo(*TypeProvider::stringMemory());

		if (notConvertibleToString)
			m_errorReporter.typeError(
				9977_error,
				argument->location(),
				"Invalid type for argument in the string.concat function call. "
				"string type is required, but " +
				argumentType->identifier() + " provided."
			);
	}
}

void TypeChecker::typeCheckBytesConcatFunction(
	FunctionCall const& _functionCall,
	FunctionType const* _functionType
)
{
	solAssert(_functionType);
	solAssert(_functionType->kind() == FunctionType::Kind::BytesConcat);
	solAssert(_functionCall.names().empty());

	typeCheckFunctionGeneralChecks(_functionCall, _functionType);

	for (shared_ptr<Expression const> const& argument: _functionCall.arguments())
	{
		Type const* argumentType = type(*argument);
		bool notConvertibleToBytes =
			!argumentType->isImplicitlyConvertibleTo(*TypeProvider::fixedBytes(32)) &&
			!argumentType->isImplicitlyConvertibleTo(*TypeProvider::bytesMemory());
		bool numberLiteral = (dynamic_cast<RationalNumberType const*>(argumentType) != nullptr);

		if (notConvertibleToBytes || numberLiteral)
			m_errorReporter.typeError(
				8015_error,
				argument->location(),
				"Invalid type for argument in the bytes.concat function call. "
				"bytes or fixed bytes type is required, but " +
				argumentType->humanReadableName() + " provided."
			);
	}
}

void TypeChecker::typeCheckFunctionGeneralChecks(
	FunctionCall const& _functionCall,
	FunctionTypePointer _functionType
)
{
	// Actual function call or struct constructor call.

	solAssert(!!_functionType, "");
	solAssert(_functionType->kind() != FunctionType::Kind::ABIDecode, "");

	bool const isPositionalCall = _functionCall.names().empty();
	bool const isVariadic = _functionType->takesArbitraryParameters();

	auto functionCallKind = *_functionCall.annotation().kind;

	solAssert(
		!isVariadic || functionCallKind == FunctionCallKind::FunctionCall,
		"Struct constructor calls cannot be variadic."
	);

	TypePointers const& parameterTypes = _functionType->parameterTypes();
	vector<ASTPointer<Expression const>> const& arguments = _functionCall.arguments();
	vector<ASTPointer<ASTString>> const& argumentNames = _functionCall.names();

	// Check number of passed in arguments
	if (
		arguments.size() < parameterTypes.size() ||
		(!isVariadic && arguments.size() > parameterTypes.size())
	)
	{
		bool const isStructConstructorCall =
			functionCallKind == FunctionCallKind::StructConstructorCall;

		auto [errorId, description] = [&]() -> tuple<ErrorId, string> {
			string msg = isVariadic ?
				"Need at least " +
				toString(parameterTypes.size()) +
				" arguments for " +
				string(isStructConstructorCall ? "struct constructor" : "function call") +
				", but provided only " +
				toString(arguments.size()) +
				"."
				:
				"Wrong argument count for " +
				string(isStructConstructorCall ? "struct constructor" : "function call") +
				": " +
				toString(arguments.size()) +
				" arguments given but " +
				string(isVariadic ? "need at least " : "expected ") +
				toString(parameterTypes.size()) +
				".";

			if (isStructConstructorCall)
			{
				solAssert(!isVariadic, "");
				return { 9755_error, msg };
			}
			else if (
				_functionType->kind() == FunctionType::Kind::BareCall ||
				_functionType->kind() == FunctionType::Kind::BareCallCode ||
				_functionType->kind() == FunctionType::Kind::BareDelegateCall ||
				_functionType->kind() == FunctionType::Kind::BareStaticCall
			)
			{
				solAssert(!isVariadic, "");
				if (arguments.empty())
					return {
						6138_error,
						msg +
						" This function requires a single bytes argument."
						" Use \"\" as argument to provide empty calldata."
					};
				else
					return {
						8922_error,
						msg +
						" This function requires a single bytes argument."
						" If all your arguments are value types, you can use"
						" abi.encode(...) to properly generate it."
					};
			}
			else if (
				_functionType->kind() == FunctionType::Kind::KECCAK256 ||
				_functionType->kind() == FunctionType::Kind::SHA256 ||
				_functionType->kind() == FunctionType::Kind::RIPEMD160
			)
			{
				solAssert(!isVariadic, "");
				return {
					4323_error,
					msg +
					" This function requires a single bytes argument."
					" Use abi.encodePacked(...) to obtain the pre-0.5.0"
					" behaviour or abi.encode(...) to use ABI encoding."
				};
			}
			else
				return { isVariadic ? 9308_error : 6160_error, msg };
		}();

		m_errorReporter.typeError(errorId, _functionCall.location(), description);
		return;
	}

	// Parameter to argument map
	std::vector<Expression const*> paramArgMap(parameterTypes.size());

	// Map parameters to arguments - trivially for positional calls, less so for named calls
	if (isPositionalCall)
		for (size_t i = 0; i < paramArgMap.size(); ++i)
			paramArgMap[i] = arguments[i].get();
	else
	{
		auto const& parameterNames = _functionType->parameterNames();

		solAssert(
			parameterNames.size() == argumentNames.size(),
			"Unexpected parameter length mismatch!"
		);

		// Check for duplicate argument names
		{
			bool duplication = false;
			for (size_t i = 0; i < argumentNames.size(); i++)
				for (size_t j = i + 1; j < argumentNames.size(); j++)
					if (*argumentNames[i] == *argumentNames[j])
					{
						duplication = true;
						m_errorReporter.typeError(
							6995_error,
							arguments[i]->location(),
							"Duplicate named argument \"" + *argumentNames[i] + "\"."
						);
					}
			if (duplication)
				return;
		}

		// map parameter names to argument names
		{
			bool not_all_mapped = false;

			for (size_t i = 0; i < argumentNames.size(); i++)
			{
				size_t j;
				for (j = 0; j < parameterNames.size(); j++)
					if (parameterNames[j] == *argumentNames[i])
						break;

				if (j < parameterNames.size())
					paramArgMap[j] = arguments[i].get();
				else
				{
					not_all_mapped = true;
					m_errorReporter.typeError(
						4974_error,
						_functionCall.location(),
						"Named argument \"" +
						*argumentNames[i] +
						"\" does not match function declaration."
					);
				}
			}

			if (not_all_mapped)
				return;
		}
	}

	// Check for compatible types between arguments and parameters
	for (size_t i = 0; i < paramArgMap.size(); ++i)
	{
		solAssert(!!paramArgMap[i], "unmapped parameter");
		BoolResult result = type(*paramArgMap[i])->isImplicitlyConvertibleTo(*parameterTypes[i]);
		if (!result)
		{
			auto [errorId, description] = [&]() -> tuple<ErrorId, string> {
				string msg =
					"Invalid type for argument in function call. "
					"Invalid implicit conversion from " +
					type(*paramArgMap[i])->humanReadableName() +
					" to " +
					parameterTypes[i]->humanReadableName() +
					" requested.";
				if (!result.message().empty())
					msg += " " + result.message();
				if (
					_functionType->kind() == FunctionType::Kind::BareCall ||
					_functionType->kind() == FunctionType::Kind::BareCallCode ||
					_functionType->kind() == FunctionType::Kind::BareDelegateCall ||
					_functionType->kind() == FunctionType::Kind::BareStaticCall
				)
					return {
						8051_error,
						msg +
						" This function requires a single bytes argument."
						" If all your arguments are value types, you can"
						" use abi.encode(...) to properly generate it."
					};
				else if (
					_functionType->kind() == FunctionType::Kind::KECCAK256 ||
					_functionType->kind() == FunctionType::Kind::SHA256 ||
					_functionType->kind() == FunctionType::Kind::RIPEMD160
				)
					return {
						7556_error,
						msg +
						" This function requires a single bytes argument."
						" Use abi.encodePacked(...) to obtain the pre-0.5.0"
						" behaviour or abi.encode(...) to use ABI encoding."
					};
				else
					return { 9553_error, msg };
			}();
			m_errorReporter.typeError(errorId, paramArgMap[i]->location(), description);
		}
	}

	TypePointers const& returnParameterTypes = _functionType->returnParameterTypes();
	bool isLibraryCall = (_functionType->kind() == FunctionType::Kind::DelegateCall);
	bool callRequiresABIEncoding =
		// ABIEncode/ABIDecode calls not included because they should have been already validated
		// at this point and they have variadic arguments so they need special handling.
		_functionType->kind() == FunctionType::Kind::DelegateCall ||
		_functionType->kind() == FunctionType::Kind::External ||
		_functionType->kind() == FunctionType::Kind::Creation ||
		_functionType->kind() == FunctionType::Kind::Event ||
		_functionType->kind() == FunctionType::Kind::Error;

	if (callRequiresABIEncoding && !useABICoderV2())
	{
		solAssert(!isVariadic, "");
		solAssert(parameterTypes.size() == arguments.size(), "");
		solAssert(!_functionType->isBareCall(), "");
		solAssert(*_functionCall.annotation().kind == FunctionCallKind::FunctionCall, "");

		for (size_t i = 0; i < parameterTypes.size(); ++i)
		{
			solAssert(parameterTypes[i], "");

			if (!typeSupportedByOldABIEncoder(*parameterTypes[i], isLibraryCall))
				m_errorReporter.typeError(
					2443_error,
					paramArgMap[i]->location(),
					"The type of this parameter, " + parameterTypes[i]->humanReadableName() + ", "
					"is only supported in ABI coder v2. "
					"Use \"pragma abicoder v2;\" to enable the feature."
				);
		}

		for (size_t i = 0; i < returnParameterTypes.size(); ++i)
		{
			solAssert(returnParameterTypes[i], "");

			if (!typeSupportedByOldABIEncoder(*returnParameterTypes[i], isLibraryCall))
				m_errorReporter.typeError(
					2428_error,
					_functionCall.location(),
					"The type of return parameter " + toString(i + 1) + ", " + returnParameterTypes[i]->humanReadableName() + ", "
					"is only supported in ABI coder v2. "
					"Use \"pragma abicoder v2;\" to enable the feature."
				);
		}
	}
}

bool TypeChecker::visit(FunctionCall const& _functionCall)
{
	vector<ASTPointer<Expression const>> const& arguments = _functionCall.arguments();
	bool argumentsArePure = true;

	// We need to check arguments' type first as they will be needed for overload resolution.
	for (ASTPointer<Expression const> const& argument: arguments)
	{
		argument->accept(*this);
		if (!*argument->annotation().isPure)
			argumentsArePure = false;
	}

	// Store argument types - and names if given - for overload resolution
	{
		FuncCallArguments funcCallArgs;

		funcCallArgs.names = _functionCall.names();

		for (ASTPointer<Expression const> const& argument: arguments)
			funcCallArgs.types.push_back(type(*argument));

		_functionCall.expression().annotation().arguments = std::move(funcCallArgs);
	}

	_functionCall.expression().accept(*this);

	Type const* expressionType = type(_functionCall.expression());

	// Determine function call kind and function type for this FunctionCall node
	FunctionCallAnnotation& funcCallAnno = _functionCall.annotation();
	FunctionTypePointer functionType = nullptr;
	funcCallAnno.isConstant = false;

	bool isLValue = false;

	// Determine and assign function call kind, lvalue, purity and function type for this FunctionCall node
	switch (expressionType->category())
	{
	case Type::Category::Function:
		functionType = dynamic_cast<FunctionType const*>(expressionType);
		funcCallAnno.kind = FunctionCallKind::FunctionCall;

		if (auto memberAccess = dynamic_cast<MemberAccess const*>(&_functionCall.expression()))
		{
			if (dynamic_cast<FunctionDefinition const*>(memberAccess->annotation().referencedDeclaration))
				_functionCall.expression().annotation().calledDirectly = true;
		}
		else if (auto identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
			if (dynamic_cast<FunctionDefinition const*>(identifier->annotation().referencedDeclaration))
				_functionCall.expression().annotation().calledDirectly = true;

		// Purity for function calls also depends upon the callee and its FunctionType
		funcCallAnno.isPure =
			argumentsArePure &&
			*_functionCall.expression().annotation().isPure &&
			functionType->isPure();

		if (functionType->kind() == FunctionType::Kind::ArrayPush)
			isLValue = functionType->parameterTypes().empty();

		break;

	case Type::Category::TypeType:
	{
		// Determine type for type conversion or struct construction expressions
		Type const* actualType = dynamic_cast<TypeType const&>(*expressionType).actualType();
		solAssert(!!actualType, "");

		if (actualType->category() == Type::Category::Struct)
		{
			if (actualType->containsNestedMapping())
				m_errorReporter.fatalTypeError(
					9515_error,
					_functionCall.location(),
					"Struct containing a (nested) mapping cannot be constructed."
				);
			functionType = dynamic_cast<StructType const&>(*actualType).constructorType();
			funcCallAnno.kind = FunctionCallKind::StructConstructorCall;
		}
		else
		{
			if (auto const* contractType = dynamic_cast<ContractType const*>(actualType))
				if (contractType->isSuper())
					m_errorReporter.fatalTypeError(
						1744_error,
						_functionCall.location(),
						"Cannot convert to the super type."
					);
			funcCallAnno.kind = FunctionCallKind::TypeConversion;
		}

		funcCallAnno.isPure = argumentsArePure;

		break;
	}

	default:
		m_errorReporter.fatalTypeError(5704_error, _functionCall.location(), "Type is not callable");
		// Unreachable, because fatalTypeError throws. We don't set kind, but that's okay because the switch below
		// is never reached. And, even if it was, SetOnce would trigger an assertion violation and not UB.
		funcCallAnno.isPure = argumentsArePure;
		break;
	}

	funcCallAnno.isLValue = isLValue;

	// Determine return types
	switch (*funcCallAnno.kind)
	{
	case FunctionCallKind::TypeConversion:
		funcCallAnno.type = typeCheckTypeConversionAndRetrieveReturnType(_functionCall);
		break;

	case FunctionCallKind::StructConstructorCall: // fall-through
	case FunctionCallKind::FunctionCall:
	{
		TypePointers returnTypes;

		switch (functionType->kind())
		{
		case FunctionType::Kind::ABIDecode:
		{
			returnTypes = typeCheckABIDecodeAndRetrieveReturnType(
				_functionCall,
				useABICoderV2()
			);
			break;
		}
		case FunctionType::Kind::ABIEncode:
		case FunctionType::Kind::ABIEncodePacked:
		case FunctionType::Kind::ABIEncodeWithSelector:
		case FunctionType::Kind::ABIEncodeWithSignature:
		case FunctionType::Kind::ABIEncodeCall:
		{
			typeCheckABIEncodeFunctions(_functionCall, functionType);
			returnTypes = functionType->returnParameterTypes();
			break;
		}
		case FunctionType::Kind::MetaType:
			returnTypes = typeCheckMetaTypeFunctionAndRetrieveReturnType(_functionCall);
			break;
		case FunctionType::Kind::BytesConcat:
		{
			typeCheckBytesConcatFunction(_functionCall, functionType);
			returnTypes = functionType->returnParameterTypes();
			break;
		}
		case FunctionType::Kind::StringConcat:
		{
			typeCheckStringConcatFunction(_functionCall, functionType);
			returnTypes = functionType->returnParameterTypes();
			break;
		}
		case FunctionType::Kind::Wrap:
		case FunctionType::Kind::Unwrap:
		{
			typeCheckFunctionGeneralChecks(_functionCall, functionType);
			returnTypes = functionType->returnParameterTypes();
			break;
		}
		default:
		{
			typeCheckFunctionCall(_functionCall, functionType);
			returnTypes = m_evmVersion.supportsReturndata() ?
				functionType->returnParameterTypes() :
				functionType->returnParameterTypesWithoutDynamicTypes();
			break;
		}
		}

		funcCallAnno.type = returnTypes.size() == 1 ?
			std::move(returnTypes.front()) :
			TypeProvider::tuple(std::move(returnTypes));

		break;
	}

	default:
		// for non-callables, ensure error reported and annotate node to void function
		solAssert(m_errorReporter.hasErrors(), "");
		funcCallAnno.kind = FunctionCallKind::FunctionCall;
		funcCallAnno.type = TypeProvider::emptyTuple();
		break;
	}

	return false;
}

bool TypeChecker::visit(FunctionCallOptions const& _functionCallOptions)
{
	solAssert(_functionCallOptions.options().size() == _functionCallOptions.names().size(), "Lengths of name & value arrays differ!");

	_functionCallOptions.expression().annotation().arguments = _functionCallOptions.annotation().arguments;

	_functionCallOptions.expression().accept(*this);

	_functionCallOptions.annotation().isPure = false;
	_functionCallOptions.annotation().isConstant = false;
	_functionCallOptions.annotation().isLValue = false;

	auto expressionFunctionType = dynamic_cast<FunctionType const*>(type(_functionCallOptions.expression()));
	if (!expressionFunctionType)
	{
		m_errorReporter.fatalTypeError(2622_error, _functionCallOptions.location(), "Expected callable expression before call options.");
		return false;
	}

	bool setSalt = false;
	bool setValue = false;
	bool setGas = false;

	FunctionType::Kind kind = expressionFunctionType->kind();
	if (
		kind != FunctionType::Kind::Creation &&
		kind != FunctionType::Kind::External &&
		kind != FunctionType::Kind::BareCall &&
		kind != FunctionType::Kind::BareCallCode &&
		kind != FunctionType::Kind::BareDelegateCall &&
		kind != FunctionType::Kind::BareStaticCall
	)
	{
		m_errorReporter.fatalTypeError(
			2193_error,
			_functionCallOptions.location(),
			"Function call options can only be set on external function calls or contract creations."
		);
		return false;
	}

	if (
		expressionFunctionType->valueSet() ||
		expressionFunctionType->gasSet() ||
		expressionFunctionType->saltSet()
	)
		m_errorReporter.typeError(
			1645_error,
			_functionCallOptions.location(),
			"Function call options have already been set, you have to combine them into a single "
			"{...}-option."
		);

	auto setCheckOption = [&](bool& _option, string const& _name)
	{
		if (_option)
			m_errorReporter.typeError(
				9886_error,
				_functionCallOptions.location(),
				"Duplicate option \"" + std::move(_name) + "\"."
			);

		_option = true;
	};

	for (size_t i = 0; i < _functionCallOptions.names().size(); ++i)
	{
		string const& name = *(_functionCallOptions.names()[i]);
		if (name == "salt")
		{
			if (kind == FunctionType::Kind::Creation)
			{
				setCheckOption(setSalt, "salt");
				expectType(*_functionCallOptions.options()[i], *TypeProvider::fixedBytes(32));
			}
			else
				m_errorReporter.typeError(
					2721_error,
					_functionCallOptions.location(),
					"Function call option \"salt\" can only be used with \"new\"."
				);
		}
		else if (name == "value")
		{
			if (kind == FunctionType::Kind::BareDelegateCall)
				m_errorReporter.typeError(
					6189_error,
					_functionCallOptions.location(),
					"Cannot set option \"value\" for delegatecall."
				);
			else if (kind == FunctionType::Kind::BareStaticCall)
				m_errorReporter.typeError(
					2842_error,
					_functionCallOptions.location(),
					"Cannot set option \"value\" for staticcall."
				);
			else if (!expressionFunctionType->isPayable())
				m_errorReporter.typeError(
					7006_error,
					_functionCallOptions.location(),
					kind == FunctionType::Kind::Creation ?
						"Cannot set option \"value\", since the constructor of " +
						expressionFunctionType->returnParameterTypes().front()->humanReadableName() +
						" is not payable." :
						"Cannot set option \"value\" on a non-payable function type."
				);
			else
			{
				expectType(*_functionCallOptions.options()[i], *TypeProvider::uint256());

				setCheckOption(setValue, "value");
			}
		}
		else if (name == "gas")
		{
			if (kind == FunctionType::Kind::Creation)
				m_errorReporter.typeError(
					9903_error,
					_functionCallOptions.location(),
					"Function call option \"gas\" cannot be used with \"new\"."
				);
			else
			{
				expectType(*_functionCallOptions.options()[i], *TypeProvider::uint256());

				setCheckOption(setGas, "gas");
			}
		}
		else
			m_errorReporter.typeError(
				9318_error,
				_functionCallOptions.location(),
				"Unknown call option \"" + name + "\". Valid options are \"salt\", \"value\" and \"gas\"."
			);
	}

	if (setSalt && !m_evmVersion.hasCreate2())
		m_errorReporter.typeError(
			5189_error,
			_functionCallOptions.location(),
			"Unsupported call option \"salt\" (requires Constantinople-compatible VMs)."
		);

	_functionCallOptions.annotation().type = expressionFunctionType->copyAndSetCallOptions(setGas, setValue, setSalt);
	return false;
}

void TypeChecker::endVisit(NewExpression const& _newExpression)
{
	Type const* type = _newExpression.typeName().annotation().type;
	solAssert(!!type, "Type name not resolved.");

	_newExpression.annotation().isConstant = false;
	_newExpression.annotation().isLValue = false;

	if (auto contractName = dynamic_cast<UserDefinedTypeName const*>(&_newExpression.typeName()))
	{
		auto contract = dynamic_cast<ContractDefinition const*>(&dereference(contractName->pathNode()));

		if (!contract)
			m_errorReporter.fatalTypeError(5540_error, _newExpression.location(), "Identifier is not a contract.");
		if (contract->isInterface())
			m_errorReporter.fatalTypeError(2971_error, _newExpression.location(), "Cannot instantiate an interface.");
		if (contract->abstract())
			m_errorReporter.typeError(4614_error, _newExpression.location(), "Cannot instantiate an abstract contract.");

		_newExpression.annotation().type = FunctionType::newExpressionType(*contract);
		_newExpression.annotation().isPure = false;
	}
	else if (type->category() == Type::Category::Array)
	{
		if (type->containsNestedMapping())
			m_errorReporter.fatalTypeError(
				1164_error,
				_newExpression.typeName().location(),
				"Array containing a (nested) mapping cannot be constructed in memory."
			);
		if (!type->isDynamicallySized())
			m_errorReporter.typeError(
				3904_error,
				_newExpression.typeName().location(),
				"Length has to be placed in parentheses after the array type for new expression."
			);
		type = TypeProvider::withLocationIfReference(DataLocation::Memory, type);
		_newExpression.annotation().type = TypeProvider::function(
			TypePointers{TypeProvider::uint256()},
			TypePointers{type},
			strings(1, ""),
			strings(1, ""),
			FunctionType::Kind::ObjectCreation,
			StateMutability::Pure
		);
		_newExpression.annotation().isPure = true;
	}
	else
	{
		_newExpression.annotation().isPure = false;
		m_errorReporter.fatalTypeError(8807_error, _newExpression.location(), "Contract or array type expected.");
	}
}

bool TypeChecker::visit(MemberAccess const& _memberAccess)
{
	_memberAccess.expression().accept(*this);
	Type const* exprType = type(_memberAccess.expression());
	ASTString const& memberName = _memberAccess.memberName();

	auto& annotation = _memberAccess.annotation();

	// Retrieve the types of the arguments if this is used to call a function.
	auto const& arguments = annotation.arguments;
	MemberList::MemberMap possibleMembers = exprType->members(currentDefinitionScope()).membersByName(memberName);
	size_t const initialMemberCount = possibleMembers.size();
	if (initialMemberCount > 1 && arguments)
	{
		// do overload resolution
		for (auto it = possibleMembers.begin(); it != possibleMembers.end();)
			if (
				it->type->category() == Type::Category::Function &&
				!dynamic_cast<FunctionType const&>(*it->type).canTakeArguments(*arguments, exprType)
			)
				it = possibleMembers.erase(it);
			else
				++it;
	}

	annotation.isConstant = false;

	if (possibleMembers.empty())
	{
		if (initialMemberCount == 0 && !dynamic_cast<ArraySliceType const*>(exprType))
		{
			// Try to see if the member was removed because it is only available for storage types.
			auto storageType = TypeProvider::withLocationIfReference(
				DataLocation::Storage,
				exprType
			);
			if (!storageType->members(currentDefinitionScope()).membersByName(memberName).empty())
				m_errorReporter.fatalTypeError(
					4994_error,
					_memberAccess.location(),
					"Member \"" + memberName + "\" is not available in " +
					exprType->humanReadableName() +
					" outside of storage."
				);
		}

		auto [errorId, description] = [&]() -> tuple<ErrorId, string> {
			string errorMsg = "Member \"" + memberName + "\" not found or not visible "
				"after argument-dependent lookup in " + exprType->humanReadableName() + ".";

			if (auto const* funType = dynamic_cast<FunctionType const*>(exprType))
			{
				TypePointers const& t = funType->returnParameterTypes();

				if (memberName == "value")
				{
					if (funType->kind() == FunctionType::Kind::Creation)
						return {
							8827_error,
							"Constructor for " + t.front()->humanReadableName() + " must be payable for member \"value\" to be available."
						};
					else if (
						funType->kind() == FunctionType::Kind::DelegateCall ||
						funType->kind() == FunctionType::Kind::BareDelegateCall
					)
						return { 8477_error, "Member \"value\" is not allowed in delegated calls due to \"msg.value\" persisting." };
					else
						return { 8820_error, "Member \"value\" is only available for payable functions." };
				}
				else if (
					t.size() == 1 && (
						t.front()->category() == Type::Category::Struct ||
						t.front()->category() == Type::Category::Contract
					)
				)
					return { 6005_error, errorMsg + " Did you intend to call the function?" };
			}
			else if (exprType->category() == Type::Category::Contract)
			{
				for (MemberList::Member const& addressMember: TypeProvider::payableAddress()->nativeMembers(nullptr))
					if (addressMember.name == memberName)
					{
						auto const* var = dynamic_cast<Identifier const*>(&_memberAccess.expression());
						string varName = var ? var->name() : "...";
						errorMsg += " Use \"address(" + varName + ")." + memberName + "\" to access this address member.";
						return { 3125_error, errorMsg };
					}
			}
			else if (auto const* addressType = dynamic_cast<AddressType const*>(exprType))
			{
				// Trigger error when using send or transfer with a non-payable fallback function.
				if (memberName == "send" || memberName == "transfer")
				{
					solAssert(
						addressType->stateMutability() != StateMutability::Payable,
						"Expected address not-payable as members were not found"
					);

					return { 9862_error, "\"send\" and \"transfer\" are only available for objects of type \"address payable\", not \"" + exprType->humanReadableName() + "\"." };
				}
			}

			return { 9582_error, errorMsg };
		}();

		m_errorReporter.fatalTypeError(
			errorId,
			_memberAccess.location(),
			description
		);
	}
	else if (possibleMembers.size() > 1)
		m_errorReporter.fatalTypeError(
			6675_error,
			_memberAccess.location(),
			"Member \"" + memberName + "\" not unique "
			"after argument-dependent lookup in " + exprType->humanReadableName() +
			(memberName == "value" ? " - did you forget the \"payable\" modifier?" : ".")
		);

	annotation.referencedDeclaration = possibleMembers.front().declaration;
	annotation.type = possibleMembers.front().type;

	VirtualLookup requiredLookup = VirtualLookup::Static;

	if (auto funType = dynamic_cast<FunctionType const*>(annotation.type))
	{
		solAssert(
			!funType->bound() || exprType->isImplicitlyConvertibleTo(*funType->selfType()),
			"Function \"" + memberName + "\" cannot be called on an object of type " +
			exprType->humanReadableName() + " (expected " + funType->selfType()->humanReadableName() + ")."
		);

		if (
			dynamic_cast<FunctionType const*>(exprType) &&
			!annotation.referencedDeclaration &&
			(memberName == "value" || memberName == "gas")
		)
			m_errorReporter.typeError(
				1621_error,
				_memberAccess.location(),
				"Using \"." + memberName + "(...)\" is deprecated. Use \"{" + memberName + ": ...}\" instead."
			);

		if (
			funType->kind() == FunctionType::Kind::ArrayPush &&
			arguments.value().numArguments() != 0 &&
			exprType->containsNestedMapping()
		)
			m_errorReporter.typeError(
				8871_error,
				_memberAccess.location(),
				"Storage arrays with nested mappings do not support .push(<arg>)."
			);

		if (!funType->bound())
			if (auto typeType = dynamic_cast<TypeType const*>(exprType))
			{
				auto contractType = dynamic_cast<ContractType const*>(typeType->actualType());
				if (contractType && contractType->isSuper())
					requiredLookup = VirtualLookup::Super;
			}
	}

	annotation.requiredLookup = requiredLookup;

	if (auto const* structType = dynamic_cast<StructType const*>(exprType))
		annotation.isLValue = !structType->dataStoredIn(DataLocation::CallData);
	else if (exprType->category() == Type::Category::Array)
		annotation.isLValue = false;
	else if (exprType->category() == Type::Category::FixedBytes)
		annotation.isLValue = false;
	else if (TypeType const* typeType = dynamic_cast<decltype(typeType)>(exprType))
	{
		if (ContractType const* contractType = dynamic_cast<decltype(contractType)>(typeType->actualType()))
		{
			annotation.isLValue = annotation.referencedDeclaration->isLValue();
			if (
				auto const* functionType = dynamic_cast<FunctionType const*>(annotation.type);
				functionType &&
				functionType->kind() == FunctionType::Kind::Declaration
			)
				annotation.isPure = *_memberAccess.expression().annotation().isPure;
		}
		else
			annotation.isLValue = false;
	}
	else if (exprType->category() == Type::Category::Module)
	{
		annotation.isPure = *_memberAccess.expression().annotation().isPure;
		annotation.isLValue = false;
	}
	else
		annotation.isLValue = false;

	// TODO some members might be pure, but for example `address(0x123).balance` is not pure
	// although every subexpression is, so leaving this limited for now.
	if (auto tt = dynamic_cast<TypeType const*>(exprType))
		if (
			tt->actualType()->category() == Type::Category::Enum ||
			tt->actualType()->category() == Type::Category::UserDefinedValueType
		)
			annotation.isPure = true;
	if (
		auto const* functionType = dynamic_cast<FunctionType const*>(exprType);
		functionType &&
		functionType->hasDeclaration() &&
		dynamic_cast<FunctionDefinition const*>(&functionType->declaration()) &&
		memberName == "selector"
	)
		if (auto const* parentAccess = dynamic_cast<MemberAccess const*>(&_memberAccess.expression()))
		{
			bool isPure = *parentAccess->expression().annotation().isPure;
			if (auto const* exprInt = dynamic_cast<Identifier const*>(&parentAccess->expression()))
				if (exprInt->name() == "this" || exprInt->name() == "super")
					isPure = true;

			annotation.isPure = isPure;
		}
	if (
		auto const* varDecl = dynamic_cast<VariableDeclaration const*>(annotation.referencedDeclaration);
		!annotation.isPure.set() &&
		varDecl &&
		varDecl->isConstant()
	)
		annotation.isPure = true;

	if (auto magicType = dynamic_cast<MagicType const*>(exprType))
	{
		if (magicType->kind() == MagicType::Kind::ABI)
			annotation.isPure = true;
		else if (magicType->kind() == MagicType::Kind::MetaType && (
			memberName == "creationCode" || memberName == "runtimeCode"
		))
		{
			annotation.isPure = true;
			ContractType const& accessedContractType = dynamic_cast<ContractType const&>(*magicType->typeArgument());
			solAssert(!accessedContractType.isSuper(), "");
			if (
				memberName == "runtimeCode" &&
				!accessedContractType.immutableVariables().empty()
			)
				m_errorReporter.typeError(
					9274_error,
					_memberAccess.location(),
					"\"runtimeCode\" is not available for contracts containing immutable variables."
				);
		}
		else if (magicType->kind() == MagicType::Kind::MetaType && memberName == "name")
			annotation.isPure = true;
		else if (magicType->kind() == MagicType::Kind::MetaType && memberName == "interfaceId")
			annotation.isPure = true;
		else if (
			magicType->kind() == MagicType::Kind::MetaType &&
			(memberName == "min" ||	memberName == "max")
		)
			annotation.isPure = true;
		else if (magicType->kind() == MagicType::Kind::Block && memberName == "chainid" && !m_evmVersion.hasChainID())
			m_errorReporter.typeError(
				3081_error,
				_memberAccess.location(),
				"\"chainid\" is not supported by the VM version."
			);
		else if (magicType->kind() == MagicType::Kind::Block && memberName == "basefee" && !m_evmVersion.hasBaseFee())
			m_errorReporter.typeError(
				5921_error,
				_memberAccess.location(),
				"\"basefee\" is not supported by the VM version."
			);
	}

	if (
		_memberAccess.expression().annotation().type->category() == Type::Category::Address &&
		memberName == "codehash" &&
		!m_evmVersion.hasExtCodeHash()
	)
		m_errorReporter.typeError(
			7598_error,
			_memberAccess.location(),
			"\"codehash\" is not supported by the VM version."
		);

	if (!annotation.isPure.set())
		annotation.isPure = false;

	return false;
}

bool TypeChecker::visit(IndexAccess const& _access)
{
	_access.annotation().isConstant = false;
	_access.baseExpression().accept(*this);
	Type const* baseType = type(_access.baseExpression());
	Type const* resultType = nullptr;
	bool isLValue = false;
	bool isPure = *_access.baseExpression().annotation().isPure;
	Expression const* index = _access.indexExpression();
	switch (baseType->category())
	{
	case Type::Category::ArraySlice:
	{
		auto const& arrayType = dynamic_cast<ArraySliceType const&>(*baseType).arrayType();
		if (arrayType.location() != DataLocation::CallData || !arrayType.isDynamicallySized())
			m_errorReporter.typeError(4802_error, _access.location(), "Index access is only implemented for slices of dynamic calldata arrays.");
		baseType = &arrayType;
		[[fallthrough]];
	}
	case Type::Category::Array:
	{
		ArrayType const& actualType = dynamic_cast<ArrayType const&>(*baseType);
		if (!index)
			m_errorReporter.typeError(9689_error, _access.location(), "Index expression cannot be omitted.");
		else if (actualType.isString())
		{
			m_errorReporter.typeError(9961_error, _access.location(), "Index access for string is not possible.");
			index->accept(*this);
		}
		else
		{
			expectType(*index, *TypeProvider::uint256());
			if (!m_errorReporter.hasErrors())
				if (auto numberType = dynamic_cast<RationalNumberType const*>(type(*index)))
				{
					solAssert(!numberType->isFractional(), "");
					if (!actualType.isDynamicallySized() && actualType.length() <= numberType->literalValue(nullptr))
						m_errorReporter.typeError(3383_error, _access.location(), "Out of bounds array access.");
				}
		}
		resultType = actualType.baseType();
		isLValue = actualType.location() != DataLocation::CallData;
		break;
	}
	case Type::Category::Mapping:
	{
		MappingType const& actualType = dynamic_cast<MappingType const&>(*baseType);
		if (!index)
			m_errorReporter.typeError(1267_error, _access.location(), "Index expression cannot be omitted.");
		else
			expectType(*index, *actualType.keyType());
		resultType = actualType.valueType();
		isLValue = true;
		break;
	}
	case Type::Category::TypeType:
	{
		TypeType const& typeType = dynamic_cast<TypeType const&>(*baseType);
		if (auto const* contractType = dynamic_cast<ContractType const*>(typeType.actualType()))
			if (contractType->contractDefinition().isLibrary())
				m_errorReporter.typeError(2876_error, _access.location(), "Index access for library types and arrays of libraries are not possible.");
		if (!index)
			resultType = TypeProvider::typeType(TypeProvider::array(DataLocation::Memory, typeType.actualType()));
		else
		{
			u256 length = 1;
			if (expectType(*index, *TypeProvider::uint256()))
			{
				if (auto indexValue = dynamic_cast<RationalNumberType const*>(type(*index)))
					length = indexValue->literalValue(nullptr);
				else
					m_errorReporter.fatalTypeError(3940_error, index->location(), "Integer constant expected.");
			}
			else
				solAssert(m_errorReporter.hasErrors(), "Expected errors as expectType returned false");

			resultType = TypeProvider::typeType(TypeProvider::array(
				DataLocation::Memory,
				typeType.actualType(),
				length
			));
		}
		break;
	}
	case Type::Category::FixedBytes:
	{
		FixedBytesType const& bytesType = dynamic_cast<FixedBytesType const&>(*baseType);
		if (!index)
			m_errorReporter.typeError(8830_error, _access.location(), "Index expression cannot be omitted.");
		else
		{
			if (!expectType(*index, *TypeProvider::uint256()))
				m_errorReporter.fatalTypeError(6318_error, _access.location(), "Index expression cannot be represented as an unsigned integer.");
			if (auto integerType = dynamic_cast<RationalNumberType const*>(type(*index)))
				if (bytesType.numBytes() <= integerType->literalValue(nullptr))
					m_errorReporter.typeError(1859_error, _access.location(), "Out of bounds array access.");
		}
		resultType = TypeProvider::fixedBytes(1);
		isLValue = false; // @todo this heavily depends on how it is embedded
		break;
	}
	default:
		m_errorReporter.fatalTypeError(
			2614_error,
			_access.baseExpression().location(),
			"Indexed expression has to be a type, mapping or array (is " + baseType->humanReadableName() + ")"
		);
	}
	_access.annotation().type = resultType;
	_access.annotation().isLValue = isLValue;
	if (index && !*index->annotation().isPure)
		isPure = false;
	_access.annotation().isPure = isPure;

	return false;
}

bool TypeChecker::visit(IndexRangeAccess const& _access)
{
	_access.annotation().isConstant = false;
	_access.baseExpression().accept(*this);

	bool isLValue = false; // TODO: set this correctly when implementing slices for memory and storage arrays
	bool isPure = *_access.baseExpression().annotation().isPure;

	if (Expression const* start = _access.startExpression())
	{
		expectType(*start, *TypeProvider::uint256());
		if (!*start->annotation().isPure)
			isPure = false;
	}
	if (Expression const* end = _access.endExpression())
	{
		expectType(*end, *TypeProvider::uint256());
		if (!*end->annotation().isPure)
			isPure = false;
	}

	_access.annotation().isLValue = isLValue;
	_access.annotation().isPure = isPure;

	Type const* exprType = type(_access.baseExpression());
	if (exprType->category() == Type::Category::TypeType)
	{
		m_errorReporter.typeError(1760_error, _access.location(), "Types cannot be sliced.");
		_access.annotation().type = exprType;
		return false;
	}

	ArrayType const* arrayType = nullptr;
	if (auto const* arraySlice = dynamic_cast<ArraySliceType const*>(exprType))
		arrayType = &arraySlice->arrayType();
	else if (!(arrayType = dynamic_cast<ArrayType const*>(exprType)))
		m_errorReporter.fatalTypeError(4781_error, _access.location(), "Index range access is only possible for arrays and array slices.");

	if (arrayType->location() != DataLocation::CallData || !arrayType->isDynamicallySized())
		m_errorReporter.typeError(1227_error, _access.location(), "Index range access is only supported for dynamic calldata arrays.");
	else if (arrayType->baseType()->isDynamicallyEncoded())
		m_errorReporter.typeError(2148_error, _access.location(), "Index range access is not supported for arrays with dynamically encoded base types.");
	_access.annotation().type = TypeProvider::arraySlice(*arrayType);

	return false;
}

vector<Declaration const*> TypeChecker::cleanOverloadedDeclarations(
	Identifier const& _identifier,
	vector<Declaration const*> const& _candidates
)
{
	solAssert(_candidates.size() > 1, "");
	vector<Declaration const*> uniqueDeclarations;

	for (Declaration const* declaration: _candidates)
	{
		solAssert(declaration, "");
		// the declaration is functionDefinition, eventDefinition or a VariableDeclaration while declarations > 1
		solAssert(
			dynamic_cast<FunctionDefinition const*>(declaration) ||
			dynamic_cast<EventDefinition const*>(declaration) ||
			dynamic_cast<VariableDeclaration const*>(declaration) ||
			dynamic_cast<MagicVariableDeclaration const*>(declaration),
			"Found overloading involving something not a function, event or a (magic) variable."
		);

		FunctionTypePointer functionType {declaration->functionType(false)};
		if (!functionType)
			functionType = declaration->functionType(true);
		solAssert(functionType, "Failed to determine the function type of the overloaded.");

		for (Type const* parameter: functionType->parameterTypes() + functionType->returnParameterTypes())
			if (!parameter)
				m_errorReporter.fatalDeclarationError(3893_error, _identifier.location(), "Function type can not be used in this context.");

		if (uniqueDeclarations.end() == find_if(
			uniqueDeclarations.begin(),
			uniqueDeclarations.end(),
			[&](Declaration const* d)
			{
				FunctionType const* newFunctionType = d->functionType(false);
				if (!newFunctionType)
					newFunctionType = d->functionType(true);
				return newFunctionType && functionType->hasEqualParameterTypes(*newFunctionType);
			}
		))
			uniqueDeclarations.push_back(declaration);
	}
	return uniqueDeclarations;
}

bool TypeChecker::visit(Identifier const& _identifier)
{
	IdentifierAnnotation& annotation = _identifier.annotation();

	if (!annotation.referencedDeclaration)
	{
		annotation.overloadedDeclarations = cleanOverloadedDeclarations(_identifier, annotation.candidateDeclarations);
		if (annotation.overloadedDeclarations.empty())
			m_errorReporter.fatalTypeError(7593_error, _identifier.location(), "No candidates for overload resolution found.");
		else if (annotation.overloadedDeclarations.size() == 1)
			annotation.referencedDeclaration = *annotation.overloadedDeclarations.begin();
		else if (!annotation.arguments)
		{
			// The identifier should be a public state variable shadowing other functions
			vector<Declaration const*> candidates;

			for (Declaration const* declaration: annotation.overloadedDeclarations)
			{
				if (VariableDeclaration const* variableDeclaration = dynamic_cast<decltype(variableDeclaration)>(declaration))
					candidates.push_back(declaration);
			}
			if (candidates.empty())
				m_errorReporter.fatalTypeError(2144_error, _identifier.location(), "No matching declaration found after variable lookup.");
			else if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
				m_errorReporter.fatalTypeError(7589_error, _identifier.location(), "No unique declaration found after variable lookup.");
		}
		else
		{
			vector<Declaration const*> candidates;

			for (Declaration const* declaration: annotation.overloadedDeclarations)
			{
				FunctionTypePointer functionType = declaration->functionType(true);
				solAssert(!!functionType, "Requested type not present.");
				if (functionType->canTakeArguments(*annotation.arguments))
					candidates.push_back(declaration);
			}
			if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
			{
				SecondarySourceLocation ssl;

				for (Declaration const* declaration: annotation.overloadedDeclarations)
					if (!declaration->location().isValid())
					{
						// Try to re-construct function definition
						string description;
						for (auto const& param: declaration->functionType(true)->parameterTypes())
							description += (description.empty() ? "" : ", ") + param->humanReadableName();
						description = "function " + _identifier.name() + "(" + description + ")";

						ssl.append("Candidate: " + description, declaration->location());
					}
					else
						ssl.append("Candidate:", declaration->location());
				if (candidates.empty())
					m_errorReporter.fatalTypeError(9322_error, _identifier.location(), ssl, "No matching declaration found after argument-dependent lookup.");
				else
					m_errorReporter.fatalTypeError(4487_error, _identifier.location(), ssl, "No unique declaration found after argument-dependent lookup.");
			}
		}
	}
	solAssert(
		!!annotation.referencedDeclaration,
		"Referenced declaration is null after overload resolution."
	);
	bool isConstant = false;
	annotation.isLValue = annotation.referencedDeclaration->isLValue();
	annotation.type = annotation.referencedDeclaration->type();
	solAssert(annotation.type, "Declaration referenced before type could be determined.");
	if (auto variableDeclaration = dynamic_cast<VariableDeclaration const*>(annotation.referencedDeclaration))
		annotation.isPure = isConstant = variableDeclaration->isConstant();
	else if (dynamic_cast<MagicVariableDeclaration const*>(annotation.referencedDeclaration))
		annotation.isPure = dynamic_cast<FunctionType const*>(annotation.type);
	else if (dynamic_cast<TypeType const*>(annotation.type))
		annotation.isPure = true;
	else if (dynamic_cast<ModuleType const*>(annotation.type))
		annotation.isPure = true;
	else
		annotation.isPure = false;

	annotation.isConstant = isConstant;

	annotation.requiredLookup =
		dynamic_cast<CallableDeclaration const*>(annotation.referencedDeclaration) ?
		VirtualLookup::Virtual : VirtualLookup::Static;

	// Check for deprecated function names.
	// The check is done here for the case without an actual function call.
	if (FunctionType const* fType = dynamic_cast<FunctionType const*>(_identifier.annotation().type))
	{
		if (_identifier.name() == "sha3" && fType->kind() == FunctionType::Kind::KECCAK256)
			m_errorReporter.typeError(
				3557_error,
				_identifier.location(),
				"\"sha3\" has been deprecated in favour of \"keccak256\"."
			);
		else if (_identifier.name() == "suicide" && fType->kind() == FunctionType::Kind::Selfdestruct)
			m_errorReporter.typeError(
				8050_error,
				_identifier.location(),
				"\"suicide\" has been deprecated in favour of \"selfdestruct\"."
			);
	}

	if (
		MagicVariableDeclaration const* magicVar =
		dynamic_cast<MagicVariableDeclaration const*>(annotation.referencedDeclaration)
	)
		if (magicVar->type()->category() == Type::Category::Integer)
		{
			solAssert(_identifier.name() == "now", "");
			m_errorReporter.typeError(
				7359_error,
				_identifier.location(),
				"\"now\" has been deprecated. Use \"block.timestamp\" instead."
			);
		}

	return false;
}

void TypeChecker::endVisit(IdentifierPath const& _identifierPath)
{
	if (
		dynamic_cast<CallableDeclaration const*>(_identifierPath.annotation().referencedDeclaration) &&
		_identifierPath.path().size() == 1
	)
		_identifierPath.annotation().requiredLookup = VirtualLookup::Virtual;
	else
		_identifierPath.annotation().requiredLookup = VirtualLookup::Static;
}

void TypeChecker::endVisit(UserDefinedTypeName const& _userDefinedTypeName)
{
	if (!_userDefinedTypeName.annotation().type)
		_userDefinedTypeName.annotation().type = _userDefinedTypeName.pathNode().annotation().referencedDeclaration->type();
}

void TypeChecker::endVisit(ElementaryTypeNameExpression const& _expr)
{
	_expr.annotation().type = TypeProvider::typeType(TypeProvider::fromElementaryTypeName(_expr.type().typeName(), _expr.type().stateMutability()));
	_expr.annotation().isPure = true;
	_expr.annotation().isLValue = false;
	_expr.annotation().isConstant = false;
}

void TypeChecker::endVisit(Literal const& _literal)
{
	if (_literal.looksLikeAddress())
	{
		// Assign type here if it even looks like an address. This prevents double errors for invalid addresses
		_literal.annotation().type = TypeProvider::address();

		string msg;
		if (_literal.valueWithoutUnderscores().length() != 42) // "0x" + 40 hex digits
			// looksLikeAddress enforces that it is a hex literal starting with "0x"
			msg =
				"This looks like an address but is not exactly 40 hex digits. It is " +
				to_string(_literal.valueWithoutUnderscores().length() - 2) +
				" hex digits.";
		else if (!_literal.passesAddressChecksum())
		{
			msg = "This looks like an address but has an invalid checksum.";
			if (!_literal.getChecksummedAddress().empty())
				msg += " Correct checksummed address: \"" + _literal.getChecksummedAddress() + "\".";
		}

		if (!msg.empty())
			m_errorReporter.syntaxError(
				9429_error,
				_literal.location(),
				msg +
				" If this is not used as an address, please prepend '00'. " +
				"For more information please see https://docs.soliditylang.org/en/develop/types.html#address-literals"
			);
	}

	if (_literal.isHexNumber() && _literal.subDenomination() != Literal::SubDenomination::None)
		m_errorReporter.fatalTypeError(
			5145_error,
			_literal.location(),
			"Hexadecimal numbers cannot be used with unit denominations. "
			"You can use an expression of the form \"0x1234 * 1 day\" instead."
		);

	if (_literal.subDenomination() == Literal::SubDenomination::Year)
		m_errorReporter.typeError(
			4820_error,
			_literal.location(),
			"Using \"years\" as a unit denomination is deprecated."
		);

	if (!_literal.annotation().type)
		_literal.annotation().type = TypeProvider::forLiteral(_literal);

	if (!_literal.annotation().type)
		m_errorReporter.fatalTypeError(2826_error, _literal.location(), "Invalid literal value.");

	_literal.annotation().isPure = true;
	_literal.annotation().isLValue = false;
	_literal.annotation().isConstant = false;
}

void TypeChecker::endVisit(UsingForDirective const& _usingFor)
{
	if (_usingFor.global())
	{
		if (m_currentContract || !_usingFor.typeName())
		{
			solAssert(m_errorReporter.hasErrors());
			return;
		}
		solAssert(_usingFor.typeName()->annotation().type);
		if (Declaration const* typeDefinition = _usingFor.typeName()->annotation().type->typeDefinition())
		{
			if (typeDefinition->scope() != m_currentSourceUnit)
				m_errorReporter.typeError(
					4117_error,
					_usingFor.location(),
					"Can only use \"global\" with types defined in the same source unit at file level."
				);
		}
		else
			m_errorReporter.typeError(
				8841_error,
				_usingFor.location(),
				"Can only use \"global\" with user-defined types."
			);
	}

	if (!_usingFor.usesBraces())
	{
		solAssert(_usingFor.functionsOrLibrary().size() == 1);
		ContractDefinition const* library = dynamic_cast<ContractDefinition const*>(
			_usingFor.functionsOrLibrary().front()->annotation().referencedDeclaration
		);
		solAssert(library && library->isLibrary());
		// No type checking for libraries
		return;
	}

	if (!_usingFor.typeName())
	{
		solAssert(m_errorReporter.hasErrors());
		return;
	}

	solAssert(_usingFor.typeName()->annotation().type);
	Type const* normalizedType = TypeProvider::withLocationIfReference(
		DataLocation::Storage,
		_usingFor.typeName()->annotation().type
	);
	solAssert(normalizedType);

	for (auto const& [path, operator_]: _usingFor.functionsAndOperators())
	{
		solAssert(path->annotation().referencedDeclaration);
		FunctionDefinition const& functionDefinition =
			dynamic_cast<FunctionDefinition const&>(*path->annotation().referencedDeclaration);

		solAssert(functionDefinition.type());

		if (functionDefinition.parameters().empty())
			m_errorReporter.fatalTypeError(
				4731_error,
				path->location(),
				"The function \"" + joinHumanReadable(path->path(), ".") + "\" " +
				"does not have any parameters, and therefore cannot be bound to the type \"" +
				(normalizedType ? normalizedType->humanReadableName() : "*") + "\"."
			);

		FunctionType const* functionType = dynamic_cast<FunctionType const&>(*functionDefinition.type()).asBoundFunction();
		solAssert(functionType && functionType->selfType(), "");
		BoolResult result = normalizedType->isImplicitlyConvertibleTo(
			*TypeProvider::withLocationIfReference(DataLocation::Storage, functionType->selfType())
		);
		if (!result)
			m_errorReporter.typeError(
				3100_error,
				path->location(),
				"The function \"" + joinHumanReadable(path->path(), ".") + "\" "+
				"cannot be bound to the type \"" + _usingFor.typeName()->annotation().type->humanReadableName() +
				"\" because the type cannot be implicitly converted to the first argument" +
				" of the function (\"" + functionType->selfType()->humanReadableName() + "\")" +
				(
					result.message().empty() ?
					"." :
					": " +  result.message()
				)
			);
		else if (operator_)
		{
			if (!_usingFor.typeName()->annotation().type->typeDefinition())
			{
				m_errorReporter.typeError(
					5332_error,
					path->location(),
					"Operators can only be implemented for user-defined types and not for contracts."
				);
				continue;
			}
			// "-" can be used as unary and binary operator.
			bool isUnaryNegation = (
				operator_ == Token::Sub &&
				functionType->parameterTypesIncludingSelf().size() == 1
			);
			if (
				(
					(TokenTraits::isBinaryOp(*operator_) && !isUnaryNegation) ||
					TokenTraits::isCompareOp(*operator_)
				) &&
				(
					functionType->parameterTypesIncludingSelf().size() != 2 ||
					*functionType->parameterTypesIncludingSelf().at(0) !=
					*functionType->parameterTypesIncludingSelf().at(1)
				)
			)
				m_errorReporter.typeError(
					1884_error,
					path->location(),
					"The function \"" + joinHumanReadable(path->path(), ".") + "\" "+
					"needs to have two parameters of equal type to be used for the operator " +
					TokenTraits::friendlyName(*operator_) +
					"."
				);
			if (
				(isUnaryNegation || (TokenTraits::isUnaryOp(*operator_) && *operator_ != Token::Add)) &&
				functionType->parameterTypesIncludingSelf().size() != 1
			)
				m_errorReporter.typeError(
					8112_error,
					path->location(),
					"The function \"" + joinHumanReadable(path->path(), ".") + "\" "+
					"needs to have exactly one parameter to be used for the operator " +
					TokenTraits::friendlyName(*operator_) +
					"."
				);
			Type const* expectedType =
				TokenTraits::isCompareOp(*operator_) ?
				dynamic_cast<Type const*>(TypeProvider::boolean()) :
				functionType->parameterTypesIncludingSelf().at(0);

			if (
				functionType->returnParameterTypes().size() != 1 ||
				*functionType->returnParameterTypes().front() != *expectedType
			)
				m_errorReporter.typeError(
					7743_error,
					path->location(),
					"The function \"" + joinHumanReadable(path->path(), ".") + "\" "+
					"needs to return exactly one value of type " +
					expectedType->toString(true) +
					" to be used for the operator " +
					TokenTraits::friendlyName(*operator_) +
					"."
				);
		}
	}
}

void TypeChecker::checkErrorAndEventParameters(CallableDeclaration const& _callable)
{
	string kind = dynamic_cast<EventDefinition const*>(&_callable) ? "event" : "error";
	for (ASTPointer<VariableDeclaration> const& var: _callable.parameters())
	{
		if (type(*var)->containsNestedMapping())
			m_errorReporter.fatalTypeError(
				3448_error,
				var->location(),
				"Type containing a (nested) mapping is not allowed as " + kind + " parameter type."
			);
		if (!type(*var)->interfaceType(false))
			m_errorReporter.typeError(3417_error, var->location(), "Internal or recursive type is not allowed as " + kind + " parameter type.");
		if (
			!useABICoderV2() &&
			!typeSupportedByOldABIEncoder(*type(*var), false /* isLibrary */)
		)
			m_errorReporter.typeError(
				3061_error,
				var->location(),
				"This type is only supported in ABI coder v2. "
				"Use \"pragma abicoder v2;\" to enable the feature."
			);
	}
}

Declaration const& TypeChecker::dereference(Identifier const& _identifier) const
{
	solAssert(!!_identifier.annotation().referencedDeclaration, "Declaration not stored.");
	return *_identifier.annotation().referencedDeclaration;
}

Declaration const& TypeChecker::dereference(IdentifierPath const& _path) const
{
	solAssert(!!_path.annotation().referencedDeclaration, "Declaration not stored.");
	return *_path.annotation().referencedDeclaration;
}

bool TypeChecker::expectType(Expression const& _expression, Type const& _expectedType)
{
	_expression.accept(*this);
	BoolResult result = type(_expression)->isImplicitlyConvertibleTo(_expectedType);
	if (!result)
	{
		auto errorMsg = "Type " +
			type(_expression)->humanReadableName() +
			" is not implicitly convertible to expected type " +
			_expectedType.humanReadableName();
		if (
			type(_expression)->category() == Type::Category::RationalNumber &&
			dynamic_cast<RationalNumberType const*>(type(_expression))->isFractional() &&
			type(_expression)->mobileType()
		)
		{
			if (_expectedType.operator==(*type(_expression)->mobileType()))
				m_errorReporter.typeError(
					4426_error,
					_expression.location(),
					errorMsg + ", but it can be explicitly converted."
				);
			else
				m_errorReporter.typeErrorConcatenateDescriptions(
					2326_error,
					_expression.location(),
					errorMsg +
					". Try converting to type " +
					type(_expression)->mobileType()->humanReadableName() +
					" or use an explicit conversion.",
					result.message()
				);
		}
		else
			m_errorReporter.typeErrorConcatenateDescriptions(
				7407_error,
				_expression.location(),
				errorMsg + ".",
				result.message()
			);
		return false;
	}
	return true;
}

void TypeChecker::requireLValue(Expression const& _expression, bool _ordinaryAssignment)
{
	_expression.annotation().willBeWrittenTo = true;
	_expression.annotation().lValueOfOrdinaryAssignment = _ordinaryAssignment;
	_expression.accept(*this);

	if (*_expression.annotation().isLValue)
		return;

	auto [errorId, description] = [&]() -> tuple<ErrorId, string> {
		if (*_expression.annotation().isConstant)
			return { 6520_error, "Cannot assign to a constant variable." };

		if (auto indexAccess = dynamic_cast<IndexAccess const*>(&_expression))
		{
			if (type(indexAccess->baseExpression())->category() == Type::Category::FixedBytes)
				return { 4360_error, "Single bytes in fixed bytes arrays cannot be modified." };
			else if (auto arrayType = dynamic_cast<ArrayType const*>(type(indexAccess->baseExpression())))
				if (arrayType->dataStoredIn(DataLocation::CallData))
					return { 6182_error, "Calldata arrays are read-only." };
		}

		if (auto memberAccess = dynamic_cast<MemberAccess const*>(&_expression))
		{
			if (auto structType = dynamic_cast<StructType const*>(type(memberAccess->expression())))
			{
				if (structType->dataStoredIn(DataLocation::CallData))
					return { 4156_error, "Calldata structs are read-only." };
			}
			else if (dynamic_cast<ArrayType const*>(type(memberAccess->expression())))
				if (memberAccess->memberName() == "length")
					return { 7567_error, "Member \"length\" is read-only and cannot be used to resize arrays." };
		}

		if (auto identifier = dynamic_cast<Identifier const*>(&_expression))
			if (auto varDecl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
				if (varDecl->isExternalCallableParameter() && dynamic_cast<ReferenceType const*>(identifier->annotation().type))
					return { 7128_error, "External function arguments of reference type are read-only." };

		return { 4247_error, "Expression has to be an lvalue." };
	}();

	m_errorReporter.typeError(errorId, _expression.location(), description);
}

bool TypeChecker::useABICoderV2() const
{
	solAssert(m_currentSourceUnit, "");
	if (m_currentContract)
		solAssert(m_currentSourceUnit == &m_currentContract->sourceUnit(), "");
	return *m_currentSourceUnit->annotation().useABICoderV2;

}
