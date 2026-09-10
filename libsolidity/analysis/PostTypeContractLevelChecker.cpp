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
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */

#include <libsolidity/analysis/PostTypeContractLevelChecker.h>

#include <fmt/format.h>
#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolutil/FunctionSelector.h>
#include <liblangutil/ErrorReporter.h>

#include <range/v3/view/reverse.hpp>

#include <limits>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::util;

namespace
{

class SuperMemberAccessCollector: public ASTConstVisitor
{
public:
	std::vector<MemberAccess const*> superAccesses;

	bool visit(MemberAccess const& _memberAccess) override
	{
		if (
			_memberAccess.annotation().requiredLookup.set() &&
			*_memberAccess.annotation().requiredLookup == VirtualLookup::Super
		)
			superAccesses.push_back(&_memberAccess);
		return true;
	}
};

}

bool PostTypeContractLevelChecker::check(SourceUnit const& _sourceUnit)
{
	bool noErrors = true;
	for (auto* contract: ASTNode::filteredNodes<ContractDefinition>(_sourceUnit.nodes()))
		if (!check(*contract))
			noErrors = false;
	return noErrors;
}

bool PostTypeContractLevelChecker::check(ContractDefinition const& _contract)
{
	solAssert(
		_contract.annotation().creationCallGraph.set() &&
		_contract.annotation().deployedCallGraph.set(),
		""
	);

	std::map<uint32_t, std::map<std::string, SourceLocation>> errorHashes;
	for (ErrorDefinition const* error: _contract.interfaceErrors())
	{
		std::string signature = error->functionType(true)->externalSignature();
		uint32_t hash = selectorFromSignatureU32(signature);
		// Fail if there is a different signature for the same hash.
		if (!errorHashes[hash].empty() && !errorHashes[hash].count(signature))
		{
			SourceLocation& otherLocation = errorHashes[hash].begin()->second;
			m_errorReporter.typeError(
				4883_error,
				error->nameLocation(),
				SecondarySourceLocation{}.append("This error has a different signature but the same hash: ", otherLocation),
				"Error signature hash collision for " + error->functionType(true)->externalSignature()
			);
		}
		else
			errorHashes[hash][signature] = error->location();
	}

	checkSuperCallsResolvingToExternalFunctions(_contract);

	if (_contract.storageLayoutSpecifier())
		checkStorageLayoutSpecifier(_contract);

	warnStorageLayoutBaseNearStorageEnd(_contract);

	return !Error::containsErrors(m_errorReporter.errors());
}

void PostTypeContractLevelChecker::checkSuperCallsResolvingToExternalFunctions(ContractDefinition const& _contract)
{
	// Code is only generated for the most derived contract, and only there does the linearization
	// contain the bases that a `super` call in one of them can be diverted to.
	if (_contract.abstract() || _contract.isInterface() || _contract.isLibrary())
		return;

	SuperMemberAccessCollector collector;
	// Note: linearizedBaseContracts is this contract's *own* C3 linearization, with it at its head
	for (ContractDefinition const* base: _contract.annotation().linearizedBaseContracts)
		base->accept(collector);

	// collector.superAccesses is a flat vector<MemberAccess const*> holding every super.<member>
	// expression written anywhere in the bodies of _contract and all its bases -- ordered
	// by linearization position of the owning contract, then source order within it
	for (MemberAccess const* memberAccess: collector.superAccesses)
	{
		auto const* function = dynamic_cast<FunctionDefinition const*>(memberAccess->annotation().referencedDeclaration);
		auto const* typeType = dynamic_cast<TypeType const*>(memberAccess->expression().annotation().type);
		if (!function || !typeType)
			continue;
		auto const* contractType = dynamic_cast<ContractType const*>(typeType->actualType());
		if (!contractType)
			continue;
		solAssert(contractType->isSuper());

		// contractDefinition() is where `super` was written; the target also depends on the root
		ContractDefinition const* searchStart = contractType->contractDefinition().superContract(_contract);
		solAssert(searchStart, "C3 keeps a contract's own bases after it, "
				"so a contract containing a `super` is never last.");

		// the function the super call binds to, using the lookup that resolveVirtual performs
		std::optional<FunctionDefinition const*> const targetOrNone = function->superLookupTarget(_contract, *searchStart);
		solAssert(targetOrNone, "Super lookup for function " + function->name() + " found no target.");
		FunctionDefinition const& target = **targetOrNone;
		if (target.isVisibleInDerivedContracts())
			continue;
		solAssert(target.visibility() == Visibility::External);

		m_errorReporter.typeError(
			8476_error,
			memberAccess->location(),
			SecondarySourceLocation{}
				.append("The external function is declared here:", target.location())
				.append("The linearization of this contract determines the target:", _contract.nameLocation()),
			fmt::format(
				"In contract \"{}\", the \"super\" call resolves to external function \"{}.{}\", "
				"which cannot be called internally. Make \"{}.{}\" public, or change the order of "
				"base contracts in \"{}\".",
				_contract.name(),
				target.annotation().contract->name(),
				target.name(),
				target.annotation().contract->name(),
				target.name(),
				_contract.name()
			)
		);
	}
}

void PostTypeContractLevelChecker::checkStorageLayoutSpecifier(ContractDefinition const& _contract)
{
	StorageLayoutSpecifier const* storageLayoutSpecifier = _contract.storageLayoutSpecifier();
	solAssert(storageLayoutSpecifier);
	Expression const& baseSlotExpression = storageLayoutSpecifier->baseSlotExpression();

	if (!*baseSlotExpression.annotation().isPure)
	{
		m_errorReporter.typeError(
			1139_error,
			baseSlotExpression.location(),
			"The base slot of the storage layout must be a compile-time constant expression."
		);
		return;
	}

	auto const* baseSlotExpressionType = type(baseSlotExpression);
	auto const* integerType = dynamic_cast<IntegerType const*>(baseSlotExpressionType);
	auto const* rationalType = dynamic_cast<RationalNumberType const*>(baseSlotExpressionType);
	if (
		!integerType &&
		!rationalType
	)
	{
		std::string errorMsg = "The base slot of the storage layout must evaluate to an integer";
		if (dynamic_cast<AddressType const*>(baseSlotExpressionType))
			errorMsg += " (the type is 'address' instead)";
		else if (auto const* fixedBytesType = dynamic_cast<FixedBytesType const*>(baseSlotExpressionType))
			errorMsg += fmt::format(
				" (the type is 'bytes{}' instead)",
				fixedBytesType->numBytes()
				)
			;
		else if (auto const* userDefinedType = dynamic_cast<UserDefinedValueType const*>(baseSlotExpressionType))
			errorMsg += fmt::format(
				" (the type is '{}' instead)",
				userDefinedType->canonicalName()
				)
			;
		errorMsg += ".";

		m_errorReporter.typeError(
			1763_error,
			baseSlotExpression.location(),
			errorMsg
		);
		return;
	}

	rational baseSlotRationalValue;
	if (integerType)
	{
		ConstantEvaluator::TypedValue typedRational = ConstantEvaluator::evaluate(m_errorReporter, baseSlotExpression);
		solAssert(!typedRational.type || dynamic_cast<IntegerType const*>(typedRational.type));
		if (!typedRational.type)
		{
			m_errorReporter.typeError(
				1505_error,
				baseSlotExpression.location(),
				"The base slot expression contains elements that are not yet supported "
				"by the internal constant evaluator and therefore cannot be evaluated at compilation time."
			);
			return;
		}
		solAssert(std::holds_alternative<rational>(typedRational.value));
		baseSlotRationalValue = std::get<rational>(typedRational.value);
	}
	else
	{
		solAssert(rationalType);
		if (rationalType->isFractional())
		{
			m_errorReporter.typeError(
				ErrorId{1763},
				baseSlotExpression.location(),
				"The base slot of the storage layout must evaluate to an integer."
			);
			return;
		}
		baseSlotRationalValue = rationalType->value();
	}

	solAssert(baseSlotRationalValue.denominator() == 1);
	bigint baseSlot = baseSlotRationalValue.numerator();
	if (!(0 <= baseSlot && baseSlot <= std::numeric_limits<u256>::max()))
	{
		m_errorReporter.typeError(
			6753_error,
			baseSlotExpression.location(),
			fmt::format(
				"The base slot of the storage layout evaluates to {}, which is outside the range of type uint256.",
				formatNumberReadable(baseSlot)
			)
		);
		return;
	}

	storageLayoutSpecifier->annotation().baseSlot = u256(baseSlot);

	bigint size = contractStorageSizeUpperBound(_contract, VariableDeclaration::Location::Unspecified);
	solAssert(size < bigint(1) << 256);
	if (baseSlot + size >= bigint(1) << 256)
		m_errorReporter.typeError(
			5015_error,
			baseSlotExpression.location(),
			"Contract extends past the end of storage when this base slot value is specified."
		);
}

namespace
{

VariableDeclaration const* findLastStorageVariable(ContractDefinition const& _contract)
{
	for (ContractDefinition const* baseContract: _contract.annotation().linearizedBaseContracts)
	{
		auto const stateVariables = baseContract->stateVariables();
		for (VariableDeclaration const* stateVariable: stateVariables | ranges::views::reverse)
			if (
				stateVariable->referenceLocation() == VariableDeclaration::Location::Unspecified &&
				!stateVariable->isConstant() &&
				!stateVariable->immutable()
			)
				return stateVariable;
	}

	return nullptr;
}

}

void PostTypeContractLevelChecker::warnStorageLayoutBaseNearStorageEnd(ContractDefinition const& _contract)
{
	// In case of most errors the warning is pointless. E.g. if we're already past storage end.
	// If the errors were in the layout specifier, we may not even be able to get values to validate.
	if (Error::containsErrors(m_errorReporter.errors()))
		return;

	bigint storageSize = contractStorageSizeUpperBound(_contract, VariableDeclaration::Location::Unspecified);
	u256 baseSlot = layoutBaseForInheritanceHierarchy(_contract, DataLocation::Storage);
	solAssert(baseSlot + storageSize <= std::numeric_limits<u256>::max());

	if (
		u256 slotsLeft = std::numeric_limits<u256>::max() - baseSlot - u256(storageSize);
		slotsLeft <= u256(1) << 64
	)
	{
		auto const& location = _contract.storageLayoutSpecifier() ?
			_contract.storageLayoutSpecifier()->location() :
			_contract.location();

		VariableDeclaration const* lastStorageVariable = findLastStorageVariable(_contract);

		auto errorID = 3495_error;
		std::string errorMsg = "This contract is very close to the end of storage. This limits its future upgradability.";
		if (lastStorageVariable)
			m_errorReporter.warning(
				errorID,
				location,
				errorMsg,
				SecondarySourceLocation{}.append(
					fmt::format(
						"There are {} storage slots between this state variable and the end of storage.",
						formatNumberReadable(slotsLeft)
					),
					lastStorageVariable->location()
				)
			);
		else
			m_errorReporter.warning(errorID, location, errorMsg);
	}
}
