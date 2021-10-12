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

#include <libsolidity/formal/SymbolicState.h>

#include <libsolidity/formal/SymbolicTypes.h>
#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SMTEncoder.h>

#include <libsmtutil/Sorts.h>

#include <range/v3/view.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend::smt;

BlockchainVariable::BlockchainVariable(
	string _name,
	map<string, smtutil::SortPointer> _members,
	EncodingContext& _context
):
	m_name(move(_name)),
	m_members(move(_members)),
	m_context(_context)
{
	vector<string> members;
	vector<SortPointer> sorts;
	for (auto const& [component, sort]: m_members)
	{
		members.emplace_back(component);
		sorts.emplace_back(sort);
		m_componentIndices[component] = static_cast<unsigned>(members.size() - 1);
	}
	m_tuple = make_unique<SymbolicTupleVariable>(
		make_shared<smtutil::TupleSort>(m_name + "_type", members, sorts),
		m_name,
		m_context
	);
}

smtutil::Expression BlockchainVariable::member(string const& _member) const
{
	return m_tuple->component(m_componentIndices.at(_member));
}

smtutil::Expression BlockchainVariable::assignMember(string const& _member, smtutil::Expression const& _value)
{
	smtutil::Expression newTuple = smt::assignMember(m_tuple->currentValue(), {{_member, _value}});
	m_context.addAssertion(m_tuple->increaseIndex() == newTuple);
	return m_tuple->currentValue();
}

void SymbolicState::reset()
{
	m_error.resetIndex();
	m_thisAddress.resetIndex();
	m_tx.reset();
	m_crypto.reset();
	if (m_abi)
		m_abi->reset();
	/// We don't reset nor clear these pointers on purpose,
	/// since it only helps to keep the already generated types.
	if (m_state)
		m_state->reset();
}

smtutil::Expression SymbolicState::balances() const
{
	return m_state->member("balances");
}

smtutil::Expression SymbolicState::balance() const
{
	return balance(thisAddress());
}

smtutil::Expression SymbolicState::balance(smtutil::Expression _address) const
{
	return smtutil::Expression::select(balances(), move(_address));
}

smtutil::Expression SymbolicState::blockhash(smtutil::Expression _blockNumber) const
{
	return smtutil::Expression::select(m_tx.member("blockhash"), move(_blockNumber));
}

void SymbolicState::newBalances()
{
	auto tupleSort = dynamic_pointer_cast<TupleSort>(stateSort());
	auto balanceSort = tupleSort->components.at(tupleSort->memberToIndex.at("balances"));
	SymbolicVariable newBalances(balanceSort, "fresh_balances_" + to_string(m_context.newUniqueId()), m_context);
	m_state->assignMember("balances", newBalances.currentValue());
}

void SymbolicState::transfer(smtutil::Expression _from, smtutil::Expression _to, smtutil::Expression _value)
{
	unsigned indexBefore = m_state->index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_state->index();
	solAssert(indexAfter > indexBefore, "");
	m_state->newVar();
	/// Do not apply the transfer operation if _from == _to.
	auto newState = smtutil::Expression::ite(
		move(_from) == move(_to),
		m_state->value(indexBefore),
		m_state->value(indexAfter)
	);
	m_context.addAssertion(m_state->value() == newState);
}

smtutil::Expression SymbolicState::storage(ContractDefinition const& _contract) const
{
	return smt::member(m_state->member("storage"), contractStorageKey(_contract));
}

smtutil::Expression SymbolicState::storage(ContractDefinition const& _contract, smtutil::Expression _address) const
{
	return smtutil::Expression::select(storage(_contract), move(_address));
}

smtutil::Expression SymbolicState::addressActive(smtutil::Expression _address) const
{
	return smtutil::Expression::select(m_state->member("isActive"), move(_address));
}

void SymbolicState::setAddressActive(
	smtutil::Expression _address,
	bool _active
)
{
	m_state->assignMember("isActive", smtutil::Expression::store(
		m_state->member("isActive"),
		move(_address),
		smtutil::Expression(_active))
	);
}

void SymbolicState::writeStateVars(ContractDefinition const& _contract, smtutil::Expression _address)
{
	auto stateVars = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract);
	if (stateVars.empty())
		return;

	map<string, smtutil::Expression> values;
	for (auto var: stateVars)
		values.emplace(stateVarStorageKey(*var, _contract), m_context.variable(*var)->currentValue());

	smtutil::Expression thisStorage = storage(_contract, _address);
	smtutil::Expression newStorage = smt::assignMember(thisStorage, values);
	auto newContractStorage = smtutil::Expression::store(
		storage(_contract), move(_address), newStorage
	);
	smtutil::Expression newAllStorage = smt::assignMember(m_state->member("storage"), {{contractStorageKey(_contract), newContractStorage}});
	m_state->assignMember("storage", newAllStorage);
}

void SymbolicState::readStateVars(ContractDefinition const& _contract, smtutil::Expression _address)
{
	auto stateVars = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(_contract);
	if (stateVars.empty())
		return;

	auto contractStorage = storage(_contract, move(_address));
	for (auto var: stateVars)
		m_context.addAssertion(
			m_context.variable(*var)->increaseIndex() ==
			smt::member(contractStorage, stateVarStorageKey(*var, _contract))
		);
}

void SymbolicState::addBalance(smtutil::Expression _address, smtutil::Expression _value)
{
	auto newBalances = smtutil::Expression::store(
		balances(),
		_address,
		balance(_address) + move(_value)
	);
	m_state->assignMember("balances", newBalances);
}

smtutil::Expression SymbolicState::txMember(string const& _member) const
{
	return m_tx.member(_member);
}

smtutil::Expression SymbolicState::txTypeConstraints() const
{
	return
		smt::symbolicUnknownConstraints(m_tx.member("block.basefee"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.chainid"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.coinbase"), TypeProvider::address()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.difficulty"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.gaslimit"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.number"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("block.timestamp"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("msg.sender"), TypeProvider::address()) &&
		smt::symbolicUnknownConstraints(m_tx.member("msg.value"), TypeProvider::uint256()) &&
		smt::symbolicUnknownConstraints(m_tx.member("tx.origin"), TypeProvider::address()) &&
		smt::symbolicUnknownConstraints(m_tx.member("tx.gasprice"), TypeProvider::uint256());
}

smtutil::Expression SymbolicState::txNonPayableConstraint() const
{
	return m_tx.member("msg.value") == 0;
}

smtutil::Expression SymbolicState::txFunctionConstraints(FunctionDefinition const& _function) const
{
	smtutil::Expression conj = _function.isPayable() ? smtutil::Expression(true) : txNonPayableConstraint();
	if (_function.isPartOfExternalInterface())
	{
		auto sig = TypeProvider::function(_function)->externalIdentifier();
		conj = conj && m_tx.member("msg.sig") == sig;
		auto b0 = sig >> (3 * 8);
		auto b1 = (sig & 0x00ff0000) >> (2 * 8);
		auto b2 = (sig & 0x0000ff00) >> (1 * 8);
		auto b3 = (sig & 0x000000ff);
		auto data = smtutil::Expression::tuple_get(m_tx.member("msg.data"), 0);
		conj = conj && smtutil::Expression::select(data, 0) == b0;
		conj = conj && smtutil::Expression::select(data, 1) == b1;
		conj = conj && smtutil::Expression::select(data, 2) == b2;
		conj = conj && smtutil::Expression::select(data, 3) == b3;
		auto length = smtutil::Expression::tuple_get(m_tx.member("msg.data"), 1);
		// TODO add ABI size of function input parameters here \/
		conj = conj && length >= 4;
	}

	return conj;
}

void SymbolicState::prepareForSourceUnit(SourceUnit const& _source, bool _storage)
{
	auto allSources = _source.referencedSourceUnits(true);
	allSources.insert(&_source);
	set<FunctionCall const*, ASTCompareByID<FunctionCall>> abiCalls;
	set<ContractDefinition const*, ASTCompareByID<ContractDefinition>> contracts;
	for (auto const& source: allSources)
	{
		abiCalls += SMTEncoder::collectABICalls(source);
		for (auto node: source->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				contracts.insert(contract);
	}
	buildState(contracts, _storage);
	buildABIFunctions(abiCalls);
}

/// Private helpers.

string SymbolicState::contractSuffix(ContractDefinition const& _contract) const
{
	return "_" + _contract.name() + "_" + to_string(_contract.id());
}

string SymbolicState::contractStorageKey(ContractDefinition const& _contract) const
{
	return "storage" + contractSuffix(_contract);
}

string SymbolicState::stateVarStorageKey(VariableDeclaration const& _var, ContractDefinition const& _contract) const
{
	return _var.name() + "_" + to_string(_var.id()) + contractSuffix(_contract);
}

void SymbolicState::buildState(set<ContractDefinition const*, ASTCompareByID<ContractDefinition>> const& _contracts, bool _allStorages)
{
	map<string, SortPointer> stateMembers{
		{"balances", make_shared<smtutil::ArraySort>(smtutil::SortProvider::uintSort, smtutil::SortProvider::uintSort)}
	};

	if (_allStorages)
	{
		vector<string> memberNames;
		vector<SortPointer> memberSorts;
		for (auto contract: _contracts)
		{
			string suffix = contractSuffix(*contract);

			// z3 doesn't like empty tuples, so if the contract has 0
			// state vars we can't put it there.
			auto stateVars = SMTEncoder::stateVariablesIncludingInheritedAndPrivate(*contract);
			if (stateVars.empty())
				continue;

			auto names = applyMap(stateVars, [&](auto var) {
				return var->name() + "_" + to_string(var->id()) + suffix;
			});
			auto sorts = applyMap(stateVars, [](auto var) { return smtSortAbstractFunction(*var->type()); });

			string name = "storage" + suffix;
			auto storageTuple = make_shared<smtutil::TupleSort>(
				name + "_type", names, sorts
			);

			auto storageSort = make_shared<smtutil::ArraySort>(
				smtSort(*TypeProvider::address()),
				storageTuple
			);

			memberNames.emplace_back(name);
			memberSorts.emplace_back(storageSort);
		}

		stateMembers.emplace(
			"isActive",
			make_shared<smtutil::ArraySort>(smtSort(*TypeProvider::address()), smtutil::SortProvider::boolSort)
		);
		stateMembers.emplace(
			"storage",
			make_shared<smtutil::TupleSort>(
				"storage_type", memberNames, memberSorts
			)
		);
	}

	m_state = make_unique<BlockchainVariable>(
		"state",
		move(stateMembers),
		m_context
	);
}

void SymbolicState::buildABIFunctions(set<FunctionCall const*, ASTCompareByID<FunctionCall>> const& _abiFunctions)
{
	map<string, SortPointer> functions;

	for (auto const* funCall: _abiFunctions)
	{
		auto t = dynamic_cast<FunctionType const*>(funCall->expression().annotation().type);

		auto const& args = funCall->sortedArguments();
		auto const& paramTypes = t->parameterTypes();
		auto const& returnTypes = t->returnParameterTypes();


		auto argTypes = [](auto const& _args) {
			return util::applyMap(_args, [](auto arg) { return arg->annotation().type; });
		};

		/// Since each abi.* function may have a different number of input/output parameters,
		/// we generically compute those types.
		vector<frontend::Type const*> inTypes;
		vector<frontend::Type const*> outTypes;
		if (t->kind() == FunctionType::Kind::ABIDecode)
		{
			/// abi.decode : (bytes, tuple_of_types(return_types)) -> (return_types)
			solAssert(args.size() == 2, "Unexpected number of arguments for abi.decode");
			inTypes.emplace_back(TypeProvider::bytesMemory());
			auto argType = args.at(1)->annotation().type;
			if (auto const* tupleType = dynamic_cast<TupleType const*>(argType))
				for (auto componentType: tupleType->components())
				{
					auto typeType = dynamic_cast<TypeType const*>(componentType);
					solAssert(typeType, "");
					outTypes.emplace_back(typeType->actualType());
				}
			else if (auto const* typeType = dynamic_cast<TypeType const*>(argType))
				outTypes.emplace_back(typeType->actualType());
			else
				solAssert(false, "Unexpected argument of abi.decode");
		}
		else if (t->kind() == FunctionType::Kind::ABIEncodeCall)
		{
			// abi.encodeCall : (functionPointer, tuple_of_args_or_one_non_tuple_arg(arguments)) -> bytes
			solAssert(args.size() == 2, "Unexpected number of arguments for abi.encodeCall");

			outTypes.emplace_back(TypeProvider::bytesMemory());
			inTypes.emplace_back(args.at(0)->annotation().type);
			inTypes.emplace_back(args.at(1)->annotation().type);
		}
		else
		{
			outTypes = returnTypes;
			if (
				t->kind() == FunctionType::Kind::ABIEncodeWithSelector ||
				t->kind() == FunctionType::Kind::ABIEncodeWithSignature
			)
			{
				/// abi.encodeWithSelector : (bytes4, one_or_more_types) -> bytes
				/// abi.encodeWithSignature : (string, one_or_more_types) -> bytes
				inTypes.emplace_back(paramTypes.front());
				inTypes += argTypes(vector<ASTPointer<Expression const>>(args.begin() + 1, args.end()));
			}
			else
			{
				/// abi.encode/abi.encodePacked : one_or_more_types -> bytes
				solAssert(
					t->kind() == FunctionType::Kind::ABIEncode ||
					t->kind() == FunctionType::Kind::ABIEncodePacked,
					""
				);
				inTypes = argTypes(args);
			}
		}

		/// Rational numbers and string literals add the concrete values to the type name,
		/// so we replace them by uint256 and bytes since those are the same as their SMT types.
		/// TODO we could also replace all types by their ABI type.
		auto replaceTypes = [](auto& _types) {
			for (auto& t: _types)
				if (t->category() == frontend::Type::Category::RationalNumber)
					t = TypeProvider::uint256();
				else if (t->category() == frontend::Type::Category::StringLiteral)
					t = TypeProvider::bytesMemory();
				else if (auto userType = dynamic_cast<UserDefinedValueType const*>(t))
					t = &userType->underlyingType();
		};
		replaceTypes(inTypes);
		replaceTypes(outTypes);

		auto name = t->richIdentifier();
		for (auto paramType: inTypes + outTypes)
			name += "_" + paramType->richIdentifier();

		m_abiMembers[funCall] = {name, inTypes, outTypes};

		if (functions.count(name))
			continue;

		/// If there is only one input or output parameter, we use that type directly.
		/// Otherwise we create a tuple wrapping the necessary input or output types.
		auto typesToSort = [](auto const& _types, string const& _name) -> shared_ptr<Sort> {
			if (_types.size() == 1)
				return smtSortAbstractFunction(*_types.front());

			vector<string> inNames;
			vector<SortPointer> sorts;
			for (unsigned i = 0; i < _types.size(); ++i)
			{
				inNames.emplace_back(_name + "_input_" + to_string(i));
				sorts.emplace_back(smtSortAbstractFunction(*_types.at(i)));
			}
			return make_shared<smtutil::TupleSort>(
				_name + "_input",
				inNames,
				sorts
			);
		};

		auto functionSort = make_shared<smtutil::ArraySort>(
			typesToSort(inTypes, name),
			typesToSort(outTypes, name)
		);

		functions[name] = functionSort;
	}

	m_abi = make_unique<BlockchainVariable>("abi", move(functions), m_context);
}

smtutil::Expression SymbolicState::abiFunction(frontend::FunctionCall const* _funCall)
{
	solAssert(m_abi, "");
	return m_abi->member(get<0>(m_abiMembers.at(_funCall)));
}

SymbolicState::SymbolicABIFunction const& SymbolicState::abiFunctionTypes(FunctionCall const* _funCall) const
{
	return m_abiMembers.at(_funCall);
}
