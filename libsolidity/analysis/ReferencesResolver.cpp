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
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#include <libsolidity/analysis/ReferencesResolver.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>

using namespace std;
using namespace langutil;

namespace dev
{
namespace solidity
{

bool ReferencesResolver::resolve(ASTNode const& _root)
{
	_root.accept(*this);
	return !m_errorOccurred;
}

bool ReferencesResolver::visit(Block const& _block)
{
	if (!m_resolveInsideCode)
		return false;
	m_resolver.setScope(&_block);
	return true;
}

void ReferencesResolver::endVisit(Block const& _block)
{
	if (!m_resolveInsideCode)
		return;

	m_resolver.setScope(_block.scope());
}

bool ReferencesResolver::visit(ForStatement const& _for)
{
	if (!m_resolveInsideCode)
		return false;
	m_resolver.setScope(&_for);
	return true;
}

void ReferencesResolver::endVisit(ForStatement const& _for)
{
	if (!m_resolveInsideCode)
		return;
	m_resolver.setScope(_for.scope());
}

void ReferencesResolver::endVisit(VariableDeclarationStatement const& _varDeclStatement)
{
	if (!m_resolveInsideCode)
		return;
	for (auto const& var: _varDeclStatement.declarations())
		if (var)
			m_resolver.activateVariable(var->name());
}

bool ReferencesResolver::visit(Identifier const& _identifier)
{
	auto declarations = m_resolver.nameFromCurrentScope(_identifier.name());
	if (declarations.empty())
	{
		string suggestions = m_resolver.similarNameSuggestions(_identifier.name());
		string errorMessage = "Undeclared identifier.";
		if (!suggestions.empty())
		{
			if ("\"" + _identifier.name() + "\"" == suggestions)
				errorMessage += " " + std::move(suggestions) + " is not (or not yet) visible at this point.";
			else
				errorMessage += " Did you mean " + std::move(suggestions) + "?";
		}
		declarationError(_identifier.location(), errorMessage);
	}
	else if (declarations.size() == 1)
		_identifier.annotation().referencedDeclaration = declarations.front();
	else
		_identifier.annotation().overloadedDeclarations =
			m_resolver.cleanedDeclarations(_identifier, declarations);
	return false;
}

bool ReferencesResolver::visit(ElementaryTypeName const& _typeName)
{
	if (!_typeName.annotation().type)
	{
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
					m_errorReporter.typeError(
						_typeName.location(),
						"Address types can only be payable or non-payable."
					);
					break;
			}
		}
	}
	return true;
}

bool ReferencesResolver::visit(FunctionDefinition const& _functionDefinition)
{
	m_returnParameters.push_back(_functionDefinition.returnParameterList().get());
	return true;
}

void ReferencesResolver::endVisit(FunctionDefinition const&)
{
	solAssert(!m_returnParameters.empty(), "");
	m_returnParameters.pop_back();
}

bool ReferencesResolver::visit(ModifierDefinition const&)
{
	m_returnParameters.push_back(nullptr);
	return true;
}

void ReferencesResolver::endVisit(ModifierDefinition const&)
{
	solAssert(!m_returnParameters.empty(), "");
	m_returnParameters.pop_back();
}

void ReferencesResolver::endVisit(UserDefinedTypeName const& _typeName)
{
	Declaration const* declaration = m_resolver.pathFromCurrentScope(_typeName.namePath());
	if (!declaration)
	{
		fatalDeclarationError(_typeName.location(), "Identifier not found or not unique.");
		return;
	}

	_typeName.annotation().referencedDeclaration = declaration;

	if (StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(declaration))
		_typeName.annotation().type = TypeProvider::structType(*structDef, DataLocation::Storage);
	else if (EnumDefinition const* enumDef = dynamic_cast<EnumDefinition const*>(declaration))
		_typeName.annotation().type = TypeProvider::enumType(*enumDef);
	else if (ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(declaration))
		_typeName.annotation().type = TypeProvider::contract(*contract);
	else
	{
		_typeName.annotation().type = TypeProvider::emptyTuple();
		typeError(_typeName.location(), "Name has to refer to a struct, enum or contract.");
	}
}

void ReferencesResolver::endVisit(FunctionTypeName const& _typeName)
{
	switch (_typeName.visibility())
	{
	case VariableDeclaration::Visibility::Internal:
	case VariableDeclaration::Visibility::External:
		break;
	default:
		fatalTypeError(_typeName.location(), "Invalid visibility, can only be \"external\" or \"internal\".");
		return;
	}

	if (_typeName.isPayable() && _typeName.visibility() != VariableDeclaration::Visibility::External)
	{
		fatalTypeError(_typeName.location(), "Only external function types can be payable.");
		return;
	}

	if (_typeName.visibility() == VariableDeclaration::Visibility::External)
		for (auto const& t: _typeName.parameterTypes() + _typeName.returnParameterTypes())
		{
			solAssert(t->annotation().type, "Type not set for parameter.");
			if (!t->annotation().type->interfaceType(false).get())
			{
				fatalTypeError(t->location(), "Internal type cannot be used for external function type.");
				return;
			}
		}

	_typeName.annotation().type = TypeProvider::function(_typeName);
}

void ReferencesResolver::endVisit(Mapping const& _typeName)
{
	TypePointer keyType = _typeName.keyType().annotation().type;
	TypePointer valueType = _typeName.valueType().annotation().type;
	// Convert key type to memory.
	keyType = TypeProvider::withLocationIfReference(DataLocation::Memory, keyType);
	// Convert value type to storage reference.
	valueType = TypeProvider::withLocationIfReference(DataLocation::Storage, valueType);
	_typeName.annotation().type = TypeProvider::mapping(keyType, valueType);
}

void ReferencesResolver::endVisit(ArrayTypeName const& _typeName)
{
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
		if (!lengthType || !lengthType->mobileType())
			fatalTypeError(length->location(), "Invalid array length, expected integer literal or constant expression.");
		else if (lengthType->isZero())
			fatalTypeError(length->location(), "Array with zero length specified.");
		else if (lengthType->isFractional())
			fatalTypeError(length->location(), "Array with fractional length specified.");
		else if (lengthType->isNegative())
			fatalTypeError(length->location(), "Array with negative length specified.");
		else
			_typeName.annotation().type = TypeProvider::array(DataLocation::Storage, baseType, lengthType->literalValue(nullptr));
	}
	else
		_typeName.annotation().type = TypeProvider::array(DataLocation::Storage, baseType);
}

bool ReferencesResolver::visit(InlineAssembly const& _inlineAssembly)
{
	m_resolver.warnVariablesNamedLikeInstructions();

	// Errors created in this stage are completely ignored because we do not yet know
	// the type and size of external identifiers, which would result in false errors.
	// The only purpose of this step is to fill the inline assembly annotation with
	// external references.
	ErrorList errors;
	ErrorReporter errorsIgnored(errors);
	yul::ExternalIdentifierAccess::Resolver resolver =
	[&](yul::Identifier const& _identifier, yul::IdentifierContext, bool _crossesFunctionBoundary) {
		auto declarations = m_resolver.nameFromCurrentScope(_identifier.name.str());
		bool isSlot = boost::algorithm::ends_with(_identifier.name.str(), "_slot");
		bool isOffset = boost::algorithm::ends_with(_identifier.name.str(), "_offset");
		if (isSlot || isOffset)
		{
			// special mode to access storage variables
			if (!declarations.empty())
				// the special identifier exists itself, we should not allow that.
				return size_t(-1);
			string realName = _identifier.name.str().substr(0, _identifier.name.str().size() - (
				isSlot ?
				string("_slot").size() :
				string("_offset").size()
			));
			if (realName.empty())
			{
				declarationError(_identifier.location, "In variable names _slot and _offset can only be used as a suffix.");
				return size_t(-1);
			}
			declarations = m_resolver.nameFromCurrentScope(realName);
		}
		if (declarations.size() > 1)
		{
			declarationError(_identifier.location, "Multiple matching identifiers. Resolving overloaded identifiers is not supported.");
			return size_t(-1);
		}
		else if (declarations.size() == 0)
			return size_t(-1);
		if (auto var = dynamic_cast<VariableDeclaration const*>(declarations.front()))
			if (var->isLocalVariable() && _crossesFunctionBoundary)
			{
				declarationError(_identifier.location, "Cannot access local Solidity variables from inside an inline assembly function.");
				return size_t(-1);
			}
		_inlineAssembly.annotation().externalReferences[&_identifier].isSlot = isSlot;
		_inlineAssembly.annotation().externalReferences[&_identifier].isOffset = isOffset;
		_inlineAssembly.annotation().externalReferences[&_identifier].declaration = declarations.front();
		return size_t(1);
	};

	// Will be re-generated later with correct information
	// We use the latest EVM version because we will re-run it anyway.
	yul::AsmAnalysisInfo analysisInfo;
	std::optional<Error::Type> errorTypeForLoose = Error::Type::SyntaxError;
	yul::AsmAnalyzer(
		analysisInfo,
		errorsIgnored,
		errorTypeForLoose,
		yul::EVMDialect::looseAssemblyForEVM(m_evmVersion),
		resolver
	).analyze(_inlineAssembly.operations());
	return false;
}

bool ReferencesResolver::visit(Return const& _return)
{
	solAssert(!m_returnParameters.empty(), "");
	_return.annotation().functionReturnParameters = m_returnParameters.back();
	return true;
}

void ReferencesResolver::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.annotation().type)
		return;

	if (_variable.isConstant() && !_variable.isStateVariable())
		m_errorReporter.declarationError(_variable.location(), "The \"constant\" keyword can only be used for state variables.");

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
			joinHumanReadable(
				allowedDataLocations | boost::adaptors::transformed(locationToString),
				", ",
				" or "
			);
			if (_variable.isCallableParameter())
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
		typeLoc = _variable.isConstant() ? DataLocation::Memory : DataLocation::Storage;
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

void ReferencesResolver::typeError(SourceLocation const& _location, string const& _description)
{
	m_errorOccurred = true;
	m_errorReporter.typeError(_location, _description);
}

void ReferencesResolver::fatalTypeError(SourceLocation const& _location, string const& _description)
{
	m_errorOccurred = true;
	m_errorReporter.fatalTypeError(_location, _description);
}

void ReferencesResolver::declarationError(SourceLocation const& _location, string const& _description)
{
	m_errorOccurred = true;
	m_errorReporter.declarationError(_location, _description);
}

void ReferencesResolver::fatalDeclarationError(SourceLocation const& _location, string const& _description)
{
	m_errorOccurred = true;
	m_errorReporter.fatalDeclarationError(_location, _description);
}

}
}
