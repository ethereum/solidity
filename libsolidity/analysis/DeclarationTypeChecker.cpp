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

#include <libsolidity/analysis/DeclarationTypeChecker.h>

#include <libsolidity/analysis/ConstantEvaluator.h>

#include <libsolidity/ast/TypeProvider.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/Algorithms.h>

#include <boost/range/adaptor/transformed.hpp>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::frontend;

bool DeclarationTypeChecker::visit(ElementaryTypeName const& _typeName)
{
	if (_typeName.annotation().type)
		return false;

	_typeName.annotation().type = TypeProvider::fromElementaryTypeName(_typeName.typeName());
	if (_typeName.stateMutability().has_value())
	{
		// for non-address types this was already caught by the parser
		solAssert(_typeName.annotation().type->category() == Type::Category::Address, "");
		switch (*_typeName.stateMutability())
		{
			case StateMutability::Payable:
				_typeName.annotation().type = TypeProvider::payableAddress();
				break;
			case StateMutability::NonPayable:
				_typeName.annotation().type = TypeProvider::address();
				break;
			default:
				typeError(
					_typeName.location(),
					"Address types can only be payable or non-payable."
				);
				break;
		}
	}
	return true;
}

bool DeclarationTypeChecker::visit(StructDefinition const& _struct)
{
	if (_struct.annotation().recursive.has_value())
	{
		if (!m_currentStructsSeen.empty() && *_struct.annotation().recursive)
			m_recursiveStructSeen = true;
		return false;
	}

	if (m_currentStructsSeen.count(&_struct))
	{
		_struct.annotation().recursive = true;
		m_recursiveStructSeen = true;
		return false;
	}

	bool previousRecursiveStructSeen = m_recursiveStructSeen;
	bool hasRecursiveChild = false;

	m_currentStructsSeen.insert(&_struct);

	for (auto const& member: _struct.members())
	{
		m_recursiveStructSeen = false;
		member->accept(*this);
		solAssert(member->annotation().type, "");
		solAssert(member->annotation().type->canBeStored(), "Type cannot be used in struct.");
		if (m_recursiveStructSeen)
			hasRecursiveChild = true;
	}

	if (!_struct.annotation().recursive.has_value())
		_struct.annotation().recursive = hasRecursiveChild;
	m_recursiveStructSeen = previousRecursiveStructSeen || *_struct.annotation().recursive;
	m_currentStructsSeen.erase(&_struct);
	if (m_currentStructsSeen.empty())
		m_recursiveStructSeen = false;

	// Check direct recursion, fatal error if detected.
	auto visitor = [&](StructDefinition const& _struct, auto& _cycleDetector, size_t _depth)
	{
		if (_depth >= 256)
			fatalDeclarationError(_struct.location(), "Struct definition exhausts cyclic dependency validator.");

		for (ASTPointer<VariableDeclaration> const& member: _struct.members())
		{
			Type const* memberType = member->annotation().type;
			while (auto arrayType = dynamic_cast<ArrayType const*>(memberType))
			{
				if (arrayType->isDynamicallySized())
					break;
				memberType = arrayType->baseType();
			}
			if (auto structType = dynamic_cast<StructType const*>(memberType))
				if (_cycleDetector.run(structType->structDefinition()))
					return;
		}
	};
	if (util::CycleDetector<StructDefinition>(visitor).run(_struct) != nullptr)
		fatalTypeError(_struct.location(), "Recursive struct definition.");

	return false;
}

void DeclarationTypeChecker::endVisit(UserDefinedTypeName const& _typeName)
{
	if (_typeName.annotation().type)
		return;

	Declaration const* declaration = _typeName.annotation().referencedDeclaration;
	solAssert(declaration, "");

	if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
	{
		if (!m_insideFunctionType && !m_currentStructsSeen.empty())
			structDef->accept(*this);
		_typeName.annotation().type = TypeProvider::structType(*structDef, DataLocation::Storage);
	}
	else if (EnumDefinition const* enumDef = dynamic_cast<EnumDefinition const*>(declaration))
		_typeName.annotation().type = TypeProvider::enumType(*enumDef);
	else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
		_typeName.annotation().type = TypeProvider::contract(*contract);
	else
	{
		_typeName.annotation().type = TypeProvider::emptyTuple();
		fatalTypeError(_typeName.location(), "Name has to refer to a struct, enum or contract.");
	}
}
bool DeclarationTypeChecker::visit(FunctionTypeName const& _typeName)
{
	if (_typeName.annotation().type)
		return false;

	bool previousInsideFunctionType = m_insideFunctionType;
	m_insideFunctionType = true;
	_typeName.parameterTypeList()->accept(*this);
	_typeName.returnParameterTypeList()->accept(*this);
	m_insideFunctionType = previousInsideFunctionType;

	switch (_typeName.visibility())
	{
		case Visibility::Internal:
		case Visibility::External:
			break;
		default:
			fatalTypeError(_typeName.location(), "Invalid visibility, can only be \"external\" or \"internal\".");
			return false;
	}

	if (_typeName.isPayable() && _typeName.visibility() != Visibility::External)
	{
		fatalTypeError(_typeName.location(), "Only external function types can be payable.");
		return false;
	}
	_typeName.annotation().type = TypeProvider::function(_typeName);
	return false;
}
void DeclarationTypeChecker::endVisit(Mapping const& _mapping)
{
	if (_mapping.annotation().type)
		return;

	if (auto const* typeName = dynamic_cast<UserDefinedTypeName const*>(&_mapping.keyType()))
	{
		if (auto const* contractType = dynamic_cast<ContractType const*>(typeName->annotation().type))
		{
			if (contractType->contractDefinition().isLibrary())
				m_errorReporter.fatalTypeError(
					1665_error,
					typeName->location(),
					"Library types cannot be used as mapping keys."
				);
		}
		else if (typeName->annotation().type->category() != Type::Category::Enum)
			m_errorReporter.fatalTypeError(
				7804_error,
				typeName->location(),
				"Only elementary types, contract types or enums are allowed as mapping keys."
			);
	}
	else
		solAssert(dynamic_cast<ElementaryTypeName const*>(&_mapping.keyType()), "");

	TypePointer keyType = _mapping.keyType().annotation().type;
	TypePointer valueType = _mapping.valueType().annotation().type;

	// Convert key type to memory.
	keyType = TypeProvider::withLocationIfReference(DataLocation::Memory, keyType);

	// Convert value type to storage reference.
	valueType = TypeProvider::withLocationIfReference(DataLocation::Storage, valueType);
	_mapping.annotation().type = TypeProvider::mapping(keyType, valueType);
}

void DeclarationTypeChecker::endVisit(ArrayTypeName const& _typeName)
{
	if (_typeName.annotation().type)
		return;

	TypePointer baseType = _typeName.baseType().annotation().type;
	if (!baseType)
	{
		solAssert(!m_errorReporter.errors().empty(), "");
		return;
	}
	if (baseType->storageBytes() == 0)
		fatalTypeError(_typeName.baseType().location(), "Illegal base type of storage size zero for array.");
	if (Expression const* length = _typeName.length())
	{
		TypePointer& lengthTypeGeneric = length->annotation().type;
		if (!lengthTypeGeneric)
			lengthTypeGeneric = ConstantEvaluator(m_errorReporter).evaluate(*length);
		RationalNumberType const* lengthType = dynamic_cast<RationalNumberType const*>(lengthTypeGeneric);
		u256 lengthValue = 0;
		if (!lengthType || !lengthType->mobileType())
			typeError(length->location(), "Invalid array length, expected integer literal or constant expression.");
		else if (lengthType->isZero())
			typeError(length->location(), "Array with zero length specified.");
		else if (lengthType->isFractional())
			typeError(length->location(), "Array with fractional length specified.");
		else if (lengthType->isNegative())
			typeError(length->location(), "Array with negative length specified.");
		else
			lengthValue = lengthType->literalValue(nullptr);
		_typeName.annotation().type = TypeProvider::array(DataLocation::Storage, baseType, lengthValue);
	}
	else
		_typeName.annotation().type = TypeProvider::array(DataLocation::Storage, baseType);
}
void DeclarationTypeChecker::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.annotation().type)
		return;

	if (_variable.isConstant() && !_variable.isStateVariable())
		m_errorReporter.declarationError(1788_error, _variable.location(), "The \"constant\" keyword can only be used for state variables.");
	if (_variable.immutable() && !_variable.isStateVariable())
		m_errorReporter.declarationError(8297_error, _variable.location(), "The \"immutable\" keyword can only be used for state variables.");

	if (!_variable.typeName())
	{
		// This can still happen in very unusual cases where a developer uses constructs, such as
		// `var a;`, however, such code will have generated errors already.
		// However, we cannot blindingly solAssert() for that here, as the TypeChecker (which is
		// invoking ReferencesResolver) is generating it, so the error is most likely(!) generated
		// after this step.
		return;
	}
	using Location = VariableDeclaration::Location;
	Location varLoc = _variable.referenceLocation();
	DataLocation typeLoc = DataLocation::Memory;

	set<Location> allowedDataLocations = _variable.allowedDataLocations();
	if (!allowedDataLocations.count(varLoc))
	{
		auto locationToString = [](VariableDeclaration::Location _location) -> string
		{
			switch (_location)
			{
				case Location::Memory: return "\"memory\"";
				case Location::Storage: return "\"storage\"";
				case Location::CallData: return "\"calldata\"";
				case Location::Unspecified: return "none";
			}
			return {};
		};

		string errorString;
		if (!_variable.hasReferenceOrMappingType())
			errorString = "Data location can only be specified for array, struct or mapping types";
		else
		{
			errorString = "Data location must be " +
				util::joinHumanReadable(
					allowedDataLocations | boost::adaptors::transformed(locationToString),
					", ",
					" or "
				);
			if (_variable.isCallableOrCatchParameter())
				errorString +=
					" for " +
					string(_variable.isReturnParameter() ? "return " : "") +
					"parameter in" +
					string(_variable.isExternalCallableParameter() ? " external" : "") +
					" function";
			else
				errorString += " for variable";
		}
		errorString += ", but " + locationToString(varLoc) + " was given.";
		typeError(_variable.location(), errorString);

		solAssert(!allowedDataLocations.empty(), "");
		varLoc = *allowedDataLocations.begin();
	}

	// Find correct data location.
	if (_variable.isEventParameter())
	{
		solAssert(varLoc == Location::Unspecified, "");
		typeLoc = DataLocation::Memory;
	}
	else if (_variable.isStateVariable())
	{
		solAssert(varLoc == Location::Unspecified, "");
		typeLoc = (_variable.isConstant() || _variable.immutable()) ? DataLocation::Memory : DataLocation::Storage;
	}
	else if (
		dynamic_cast<StructDefinition const*>(_variable.scope()) ||
		dynamic_cast<EnumDefinition const*>(_variable.scope())
	)
		// The actual location will later be changed depending on how the type is used.
		typeLoc = DataLocation::Storage;
	else
		switch (varLoc)
		{
			case Location::Memory:
				typeLoc = DataLocation::Memory;
				break;
			case Location::Storage:
				typeLoc = DataLocation::Storage;
				break;
			case Location::CallData:
				typeLoc = DataLocation::CallData;
				break;
			case Location::Unspecified:
				solAssert(!_variable.hasReferenceOrMappingType(), "Data location not properly set.");
		}

	TypePointer type = _variable.typeName()->annotation().type;
	if (auto ref = dynamic_cast<ReferenceType const*>(type))
	{
		bool isPointer = !_variable.isStateVariable();
		type = TypeProvider::withLocation(ref, typeLoc, isPointer);
	}

	_variable.annotation().type = type;

}

void DeclarationTypeChecker::typeError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.typeError(2311_error, _location, _description);
}

void DeclarationTypeChecker::fatalTypeError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.fatalTypeError(5651_error, _location, _description);
}

void DeclarationTypeChecker::fatalDeclarationError(SourceLocation const& _location, string const& _description)
{
	m_errorReporter.fatalDeclarationError(2046_error, _location, _description);
}

bool DeclarationTypeChecker::check(ASTNode const& _node)
{
	unsigned errorCount = m_errorReporter.errorCount();
	_node.accept(*this);
	return m_errorReporter.errorCount() == errorCount;
}
