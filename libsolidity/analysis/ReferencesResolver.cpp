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
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#include <libsolidity/analysis/ReferencesResolver.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/analysis/ConstantEvaluator.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


bool ReferencesResolver::visit(Return const& _return)
{
	_return.annotation().functionReturnParameters = m_returnParameters;
	return true;
}

bool ReferencesResolver::visit(UserDefinedTypeName const& _typeName)
{
	Declaration const* declaration = m_resolver.pathFromCurrentScope(_typeName.namePath());
	if (!declaration)
		BOOST_THROW_EXCEPTION(
			Error(Error::Type::DeclarationError) <<
			errinfo_sourceLocation(_typeName.location()) <<
			errinfo_comment("Identifier not found or not unique.")
		);
	_typeName.annotation().referencedDeclaration = declaration;
	return true;
}

bool ReferencesResolver::visit(Identifier const& _identifier)
{
	auto declarations = m_resolver.nameFromCurrentScope(_identifier.name());
	if (declarations.empty())
		BOOST_THROW_EXCEPTION(
			Error(Error::Type::DeclarationError) <<
			errinfo_sourceLocation(_identifier.location()) <<
			errinfo_comment("Undeclared identifier.")
		);
	else if (declarations.size() == 1)
	{
		_identifier.annotation().referencedDeclaration = declarations.front();
		_identifier.annotation().contractScope = m_currentContract;
	}
	else
		_identifier.annotation().overloadedDeclarations =
			m_resolver.cleanedDeclarations(_identifier, declarations);
	return false;
}

void ReferencesResolver::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.annotation().type)
		return;

	TypePointer type;
	if (_variable.typeName())
	{
		type = typeFor(*_variable.typeName());
		using Location = VariableDeclaration::Location;
		Location loc = _variable.referenceLocation();
		// References are forced to calldata for external function parameters (not return)
		// and memory for parameters (also return) of publicly visible functions.
		// They default to memory for function parameters and storage for local variables.
		// As an exception, "storage" is allowed for library functions.
		if (auto ref = dynamic_cast<ReferenceType const*>(type.get()))
		{
			if (_variable.isExternalCallableParameter())
			{
				auto const& contract = dynamic_cast<ContractDefinition const&>(*_variable.scope()->scope());
				if (contract.isLibrary())
				{
					if (loc == Location::Memory)
						fatalTypeError(
							"Location has to be calldata or storage for external "
							"library functions (remove the \"memory\" keyword)."
						);
				}
				else
				{
					// force location of external function parameters (not return) to calldata
					if (loc != Location::Default)
						fatalTypeError(
							"Location has to be calldata for external functions "
							"(remove the \"memory\" or \"storage\" keyword)."
						);
				}
				if (loc == Location::Default)
					type = ref->copyForLocation(DataLocation::CallData, true);
			}
			else if (_variable.isCallableParameter() && _variable.scope()->isPublic())
			{
				auto const& contract = dynamic_cast<ContractDefinition const&>(*_variable.scope()->scope());
				// force locations of public or external function (return) parameters to memory
				if (loc == Location::Storage && !contract.isLibrary())
					fatalTypeError(
						"Location has to be memory for publicly visible functions "
						"(remove the \"storage\" keyword)."
					);
				if (loc == Location::Default || !contract.isLibrary())
					type = ref->copyForLocation(DataLocation::Memory, true);
			}
			else
			{
				if (_variable.isConstant())
				{
					if (loc != Location::Default && loc != Location::Memory)
						fatalTypeError("Storage location has to be \"memory\" (or unspecified) for constants.");
					loc = Location::Memory;
				}
				if (loc == Location::Default)
					loc = _variable.isCallableParameter() ? Location::Memory : Location::Storage;
				bool isPointer = !_variable.isStateVariable();
				type = ref->copyForLocation(
					loc == Location::Memory ?
					DataLocation::Memory :
					DataLocation::Storage,
					isPointer
				);
			}
		}
		else if (loc != Location::Default && !ref)
			fatalTypeError("Storage location can only be given for array or struct types.");

		if (!type)
			fatalTypeError("Invalid type name.");

	}
	else if (!_variable.canHaveAutoType())
		fatalTypeError("Explicit type needed.");
	// otherwise we have a "var"-declaration whose type is resolved by the first assignment

	_variable.annotation().type = type;
}

TypePointer ReferencesResolver::typeFor(TypeName const& _typeName)
{
	if (_typeName.annotation().type)
		return _typeName.annotation().type;

	TypePointer type;
	if (auto elemTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
		type = Type::fromElementaryTypeName(elemTypeName->typeName());
	else if (auto typeName = dynamic_cast<UserDefinedTypeName const*>(&_typeName))
	{
		Declaration const* declaration = typeName->annotation().referencedDeclaration;
		solAssert(!!declaration, "");

		if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
			type = make_shared<StructType>(*structDef);
		else if (EnumDefinition const* enumDef = dynamic_cast<EnumDefinition const*>(declaration))
			type = make_shared<EnumType>(*enumDef);
		else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
			type = make_shared<ContractType>(*contract);
		else
			fatalTypeError("Name has to refer to a struct, enum or contract.");
	}
	else if (auto mapping = dynamic_cast<Mapping const*>(&_typeName))
	{
		TypePointer keyType = typeFor(mapping->keyType());
		TypePointer valueType = typeFor(mapping->valueType());
		// Convert key type to memory.
		keyType = ReferenceType::copyForLocationIfReference(DataLocation::Memory, keyType);
		// Convert value type to storage reference.
		valueType = ReferenceType::copyForLocationIfReference(DataLocation::Storage, valueType);
		type = make_shared<MappingType>(keyType, valueType);
	}
	else if (auto arrayType = dynamic_cast<ArrayTypeName const*>(&_typeName))
	{
		TypePointer baseType = typeFor(arrayType->baseType());
		if (baseType->storageBytes() == 0)
			fatalTypeError("Illegal base type of storage size zero for array.");
		if (Expression const* length = arrayType->length())
		{
			if (!length->annotation().type)
				ConstantEvaluator e(*length);

			auto const* lengthType = dynamic_cast<IntegerConstantType const*>(length->annotation().type.get());
			if (!lengthType)
				fatalTypeError("Invalid array length.");
			type = make_shared<ArrayType>(DataLocation::Storage, baseType, lengthType->literalValue(nullptr));
		}
		else
			type = make_shared<ArrayType>(DataLocation::Storage, baseType);
	}

	return _typeName.annotation().type = move(type);
}

void ReferencesResolver::typeError(string const& _description)
{
	auto err = make_shared<Error>(Error::Type::TypeError);
	*err <<	errinfo_comment(_description);

	m_errors.push_back(err);
}

void ReferencesResolver::fatalTypeError(string const& _description)
{
	typeError(_description);
	BOOST_THROW_EXCEPTION(FatalError());
}


