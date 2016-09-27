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
#include <libsolidity/inlineasm/AsmCodeGen.h>
#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


bool ReferencesResolver::resolve(ASTNode const& _root)
{
	try
	{
		_root.accept(*this);
	}
	catch (FatalError const&)
	{
		solAssert(m_errorOccurred, "");
	}
	return !m_errorOccurred;
}

bool ReferencesResolver::visit(Identifier const& _identifier)
{
	auto declarations = m_resolver.nameFromCurrentScope(_identifier.name());
	if (declarations.empty())
		fatalDeclarationError(_identifier.location(), "Undeclared identifier.");
	else if (declarations.size() == 1)
		_identifier.annotation().referencedDeclaration = declarations.front();
	else
		_identifier.annotation().overloadedDeclarations =
			m_resolver.cleanedDeclarations(_identifier, declarations);
	return false;
}

bool ReferencesResolver::visit(ElementaryTypeName const& _typeName)
{
	_typeName.annotation().type = Type::fromElementaryTypeName(_typeName.typeName());
	return true;
}

void ReferencesResolver::endVisit(UserDefinedTypeName const& _typeName)
{
	Declaration const* declaration = m_resolver.pathFromCurrentScope(_typeName.namePath());
	if (!declaration)
		fatalDeclarationError(_typeName.location(), "Identifier not found or not unique.");

	_typeName.annotation().referencedDeclaration = declaration;

	if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
		_typeName.annotation().type = make_shared<StructType>(*structDef);
	else if (EnumDefinition const* enumDef = dynamic_cast<EnumDefinition const*>(declaration))
		_typeName.annotation().type = make_shared<EnumType>(*enumDef);
	else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
		_typeName.annotation().type = make_shared<ContractType>(*contract);
	else
		fatalTypeError(_typeName.location(), "Name has to refer to a struct, enum or contract.");
}

void ReferencesResolver::endVisit(FunctionTypeName const& _typeName)
{
	switch (_typeName.visibility())
	{
	case VariableDeclaration::Visibility::Default:
	case VariableDeclaration::Visibility::Internal:
	case VariableDeclaration::Visibility::External:
		break;
	default:
		typeError(_typeName.location(), "Invalid visibility, can only be \"external\" or \"internal\".");
	}

	// Do we allow storage references for external functions?

	_typeName.annotation().type = make_shared<FunctionType>(_typeName);
}

void ReferencesResolver::endVisit(Mapping const& _typeName)
{
	TypePointer keyType = _typeName.keyType().annotation().type;
	TypePointer valueType = _typeName.valueType().annotation().type;
	// Convert key type to memory.
	keyType = ReferenceType::copyForLocationIfReference(DataLocation::Memory, keyType);
	// Convert value type to storage reference.
	valueType = ReferenceType::copyForLocationIfReference(DataLocation::Storage, valueType);
	_typeName.annotation().type = make_shared<MappingType>(keyType, valueType);
}

void ReferencesResolver::endVisit(ArrayTypeName const& _typeName)
{
	TypePointer baseType = _typeName.baseType().annotation().type;
	if (baseType->storageBytes() == 0)
		fatalTypeError(_typeName.baseType().location(), "Illegal base type of storage size zero for array.");
	if (Expression const* length = _typeName.length())
	{
		if (!length->annotation().type)
			ConstantEvaluator e(*length);
		auto const* lengthType = dynamic_cast<RationalNumberType const*>(length->annotation().type.get());
		if (!lengthType || lengthType->isFractional())
			fatalTypeError(length->location(), "Invalid array length, expected integer literal.");
		else
			_typeName.annotation().type = make_shared<ArrayType>(DataLocation::Storage, baseType, lengthType->literalValue(nullptr));
	}
	else
		_typeName.annotation().type = make_shared<ArrayType>(DataLocation::Storage, baseType);
}

bool ReferencesResolver::visit(InlineAssembly const& _inlineAssembly)
{
	// We need to perform a full code generation pass here as inline assembly does not distinguish
	// reference resolution and code generation.
	// Errors created in this stage are completely ignored because we do not yet know
	// the type and size of external identifiers, which would result in false errors.
	ErrorList errorsIgnored;
	assembly::CodeGenerator codeGen(_inlineAssembly.operations(), errorsIgnored);
	codeGen.typeCheck([&](assembly::Identifier const& _identifier, eth::Assembly&, assembly::CodeGenerator::IdentifierContext) {
		auto declarations = m_resolver.nameFromCurrentScope(_identifier.name);
		if (declarations.size() != 1)
			return false;
		_inlineAssembly.annotation().externalReferences[&_identifier] = declarations.front();
		// At this stage we neither know the code to generate nor the stack size of the identifier,
		// so we do not modify assembly.
		return true;
	});
	return false;
}

bool ReferencesResolver::visit(Return const& _return)
{
	_return.annotation().functionReturnParameters = m_returnParameters;
	return true;
}

void ReferencesResolver::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.annotation().type)
		return;

	TypePointer type;
	if (_variable.typeName())
	{
		type = _variable.typeName()->annotation().type;
		using Location = VariableDeclaration::Location;
		Location varLoc = _variable.referenceLocation();
		DataLocation typeLoc = DataLocation::Memory;
		// References are forced to calldata for external function parameters (not return)
		// and memory for parameters (also return) of publicly visible functions.
		// They default to memory for function parameters and storage for local variables.
		// As an exception, "storage" is allowed for library functions.
		if (auto ref = dynamic_cast<ReferenceType const*>(type.get()))
		{
			bool isPointer = true;
			if (_variable.isExternalCallableParameter())
			{
				auto const& contract = dynamic_cast<ContractDefinition const&>(
					*dynamic_cast<Declaration const&>(*_variable.scope()).scope()
				);
				if (contract.isLibrary())
				{
					if (varLoc == Location::Memory)
						fatalTypeError(_variable.location(),
							"Location has to be calldata or storage for external "
							"library functions (remove the \"memory\" keyword)."
						);
				}
				else
				{
					// force location of external function parameters (not return) to calldata
					if (varLoc != Location::Default)
						fatalTypeError(_variable.location(),
							"Location has to be calldata for external functions "
							"(remove the \"memory\" or \"storage\" keyword)."
						);
				}
				if (varLoc == Location::Default)
					typeLoc = DataLocation::CallData;
				else
					typeLoc = varLoc == Location::Memory ? DataLocation::Memory : DataLocation::Storage;
			}
			else if (_variable.isCallableParameter() && dynamic_cast<Declaration const&>(*_variable.scope()).isPublic())
			{
				auto const& contract = dynamic_cast<ContractDefinition const&>(
					*dynamic_cast<Declaration const&>(*_variable.scope()).scope()
				);
				// force locations of public or external function (return) parameters to memory
				if (varLoc == Location::Storage && !contract.isLibrary())
					fatalTypeError(_variable.location(),
						"Location has to be memory for publicly visible functions "
						"(remove the \"storage\" keyword)."
					);
				if (varLoc == Location::Default || !contract.isLibrary())
					typeLoc = DataLocation::Memory;
				else
					typeLoc = varLoc == Location::Memory ? DataLocation::Memory : DataLocation::Storage;
			}
			else
			{
				if (_variable.isConstant())
				{
					if (varLoc != Location::Default && varLoc != Location::Memory)
						fatalTypeError(
							_variable.location(),
							"Storage location has to be \"memory\" (or unspecified) for constants."
						);
					typeLoc = DataLocation::Memory;
				}
				else if (varLoc == Location::Default)
					typeLoc = _variable.isCallableParameter() ? DataLocation::Memory : DataLocation::Storage;
				else
					typeLoc = varLoc == Location::Memory ? DataLocation::Memory : DataLocation::Storage;
				isPointer = !_variable.isStateVariable();
			}

			type = ref->copyForLocation(typeLoc, isPointer);
		}
		else if (varLoc != Location::Default && !ref)
			fatalTypeError(_variable.location(), "Storage location can only be given for array or struct types.");

		if (!type)
			fatalTypeError(_variable.location(), "Invalid type name.");

	}
	else if (!_variable.canHaveAutoType())
		fatalTypeError(_variable.location(), "Explicit type needed.");
	// otherwise we have a "var"-declaration whose type is resolved by the first assignment

	_variable.annotation().type = type;
}

void ReferencesResolver::typeError(SourceLocation const& _location, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::TypeError);
	*err <<	errinfo_sourceLocation(_location) << errinfo_comment(_description);
	m_errorOccurred = true;
	m_errors.push_back(err);
}

void ReferencesResolver::fatalTypeError(SourceLocation const& _location, string const& _description)
{
	typeError(_location, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

void ReferencesResolver::declarationError(SourceLocation const& _location, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::DeclarationError);
	*err <<	errinfo_sourceLocation(_location) << errinfo_comment(_description);
	m_errorOccurred = true;
	m_errors.push_back(err);
}

void ReferencesResolver::fatalDeclarationError(SourceLocation const& _location, string const& _description)
{
	declarationError(_location, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

