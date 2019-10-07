/*(
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
 * Base class to perform data flow analysis during AST walks.
 * Tracks assignments and is used as base class for both Rematerialiser and
 * Common Subexpression Eliminator.
 */

#include <libyul/optimiser/DataFlowAnalyzer.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/Exceptions.h>
#include <libyul/AsmData.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>

#include <libsolutil/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;


void DataFlowAnalyzer::operator()(ExpressionStatement& _statement)
{
	if (auto vars = isSimpleStore(true, _statement))
	{
		ASTModifier::operator()(_statement);
		set<YulString> keysToErase;
		for (auto const& item: m_storage.values)
			if (!(
				m_knowledgeBase.knownToBeDifferent(vars->first, item.first) ||
				m_knowledgeBase.knownToBeEqual(vars->second, item.second)
			))
				keysToErase.insert(item.first);
		for (YulString const& key: keysToErase)
			m_storage.eraseKey(key);
		m_storage.set(vars->first, vars->second);
	}
	else if (auto vars = isSimpleStore(false, _statement))
	{
		//cout << "is simple memory store" << endl;

		ASTModifier::operator()(_statement);
		set<YulString> keysToErase;
		unsigned amount = 32;
		if (dynamic_cast<WasmDialect const*>(&m_dialect))
			amount = 8;
		for (auto const& item: m_memory.values)
			if (!m_knowledgeBase.knownToBeDifferentByAtLeastConstant(vars->first, item.first, amount))
				keysToErase.insert(item.first);
		for (YulString const& key: keysToErase)
			m_memory.eraseKey(key);
		//cout << "memory[" << vars->first.str() << "] = " << vars->second.str() << endl;
		m_memory.set(vars->first, vars->second);
	}
	else
	{
		clearKnowledgeIfInvalidated(_statement.expression);
		ASTModifier::operator()(_statement);
	}
}

void DataFlowAnalyzer::operator()(Assignment& _assignment)
{
	set<YulString> names;
	for (auto const& var: _assignment.variableNames)
		names.emplace(var.name);
	assertThrow(_assignment.value, OptimizerException, "");
	clearKnowledgeIfInvalidated(*_assignment.value);
	visit(*_assignment.value);
	handleAssignment(names, _assignment.value.get());
}

void DataFlowAnalyzer::operator()(VariableDeclaration& _varDecl)
{
	set<YulString> names;
	for (auto const& var: _varDecl.variables)
		names.emplace(var.name);
	m_variableScopes.back().variables += names;

	if (_varDecl.value)
	{
		clearKnowledgeIfInvalidated(*_varDecl.value);
		visit(*_varDecl.value);
	}

	handleAssignment(names, _varDecl.value.get());
}

void DataFlowAnalyzer::operator()(If& _if)
{
	clearKnowledgeIfInvalidated(*_if.condition);
	InvertibleMap<YulString, YulString> storage = m_storage;
	InvertibleMap<YulString, YulString> memory = m_memory;

	ASTModifier::operator()(_if);

	joinKnowledge(storage, memory);

	Assignments assignments;
	assignments(_if.body);
	clearValues(assignments.names());
}

void DataFlowAnalyzer::operator()(Switch& _switch)
{
	clearKnowledgeIfInvalidated(*_switch.expression);
	visit(*_switch.expression);
	set<YulString> assignedVariables;
	for (auto& _case: _switch.cases)
	{
		InvertibleMap<YulString, YulString> storage = m_storage;
		InvertibleMap<YulString, YulString> memory = m_memory;
		(*this)(_case.body);
		joinKnowledge(storage, memory);

		Assignments assignments;
		assignments(_case.body);
		assignedVariables += assignments.names();
		// This is a little too destructive, we could retain the old values.
		clearValues(assignments.names());
		clearKnowledgeIfInvalidated(_case.body);
	}
	for (auto& _case: _switch.cases)
		clearKnowledgeIfInvalidated(_case.body);
	clearValues(assignedVariables);
}

void DataFlowAnalyzer::operator()(FunctionDefinition& _fun)
{
	// Save all information. We might rather reinstantiate this class,
	// but this could be difficult if it is subclassed.
	map<YulString, AssignedValue> value;
	size_t loopDepth{0};
	InvertibleRelation<YulString> references;
	InvertibleMap<YulString, YulString> storage;
	InvertibleMap<YulString, YulString> memory;
	swap(m_value, value);
	swap(m_loopDepth, loopDepth);
	swap(m_references, references);
	swap(m_storage, storage);
	swap(m_memory, memory);
	pushScope(true);

	for (auto const& parameter: _fun.parameters)
		m_variableScopes.back().variables.emplace(parameter.name);
	for (auto const& var: _fun.returnVariables)
	{
		m_variableScopes.back().variables.emplace(var.name);
		handleAssignment({var.name}, nullptr);
	}
	ASTModifier::operator()(_fun);

	// Note that the contents of return variables, storage and memory at this point
	// might be incorrect due to the fact that the DataFlowAnalyzer ignores the ``leave``
	// statement.

	popScope();
	swap(m_value, value);
	swap(m_loopDepth, loopDepth);
	swap(m_references, references);
	swap(m_storage, storage);
	swap(m_memory, memory);
}

void DataFlowAnalyzer::operator()(ForLoop& _for)
{
	// If the pre block was not empty,
	// we would have to deal with more complicated scoping rules.
	assertThrow(_for.pre.statements.empty(), OptimizerException, "");

	++m_loopDepth;

	AssignmentsSinceContinue assignmentsSinceCont;
	assignmentsSinceCont(_for.body);

	Assignments assignments;
	assignments(_for.body);
	assignments(_for.post);
	clearValues(assignments.names());

	// break/continue are tricky for storage and thus we almost always clear here.
	clearKnowledgeIfInvalidated(*_for.condition);
	clearKnowledgeIfInvalidated(_for.post);
	clearKnowledgeIfInvalidated(_for.body);

	visit(*_for.condition);
	(*this)(_for.body);
	clearValues(assignmentsSinceCont.names());
	clearKnowledgeIfInvalidated(_for.body);
	(*this)(_for.post);
	clearValues(assignments.names());
	clearKnowledgeIfInvalidated(*_for.condition);
	clearKnowledgeIfInvalidated(_for.post);
	clearKnowledgeIfInvalidated(_for.body);

	--m_loopDepth;
}

void DataFlowAnalyzer::operator()(Block& _block)
{
	size_t numScopes = m_variableScopes.size();
	pushScope(false);
	ASTModifier::operator()(_block);
	popScope();
	assertThrow(numScopes == m_variableScopes.size(), OptimizerException, "");
}

void DataFlowAnalyzer::handleAssignment(set<YulString> const& _variables, Expression* _value)
{
	clearValues(_variables);

	MovableChecker movableChecker{m_dialect, &m_functionSideEffects};
	if (_value)
		movableChecker.visit(*_value);
	else
		for (auto const& var: _variables)
			assignValue(var, &m_zero);

	if (_value && _variables.size() == 1)
	{
		YulString name = *_variables.begin();
		// Expression has to be movable and cannot contain a reference
		// to the variable that will be assigned to.
		if (movableChecker.movable() && !movableChecker.referencedVariables().count(name))
			assignValue(name, _value);
	}

	auto const& referencedVariables = movableChecker.referencedVariables();
	for (auto const& name: _variables)
	{
		m_references.set(name, referencedVariables);
		// assignment to slot denoted by "name"
		m_storage.eraseKey(name);
		// assignment to slot contents denoted by "name"
		m_storage.eraseValue(name);
		// assignment to slot denoted by "name"
		m_memory.eraseKey(name);
		// assignment to slot contents denoted by "name"
		m_memory.eraseValue(name);
	}
}

void DataFlowAnalyzer::pushScope(bool _functionScope)
{
	m_variableScopes.emplace_back(_functionScope);
}

void DataFlowAnalyzer::popScope()
{
	clearValues(std::move(m_variableScopes.back().variables));
	m_variableScopes.pop_back();
}

void DataFlowAnalyzer::clearValues(set<YulString> _variables)
{
	// All variables that reference variables to be cleared also have to be
	// cleared, but not recursively, since only the value of the original
	// variables changes. Example:
	// let a := 1
	// let b := a
	// let c := b
	// let a := 2
	// add(b, c)
	// In the last line, we can replace c by b, but not b by a.
	//
	// This cannot be easily tested since the substitutions will be done
	// one by one on the fly, and the last line will just be add(1, 1)

	// First clear storage knowledge, because we do not have to clear
	// storage knowledge of variables whose expression has changed,
	// since the value is still unchanged.
	for (auto const& name: _variables)
	{
		// clear slot denoted by "name"
		m_storage.eraseKey(name);
		// clear slot contents denoted by "name"
		m_storage.eraseValue(name);
		// assignment to slot denoted by "name"
		m_memory.eraseKey(name);
		// assignment to slot contents denoted by "name"
		m_memory.eraseValue(name);
	}

	// Also clear variables that reference variables to be cleared.
	for (auto const& name: _variables)
		for (auto const& ref: m_references.backward[name])
			_variables.emplace(ref);

	// Clear the value and update the reference relation.
	for (auto const& name: _variables)
		m_value.erase(name);
	for (auto const& name: _variables)
		m_references.eraseKey(name);
}

void DataFlowAnalyzer::assignValue(YulString _variable, Expression const* _value)
{
	m_value[_variable] = {_value, m_loopDepth};
}

void DataFlowAnalyzer::clearKnowledgeIfInvalidated(Block const& _block)
{
	SideEffectsCollector sideEffects(m_dialect, _block, &m_functionSideEffects);
	if (sideEffects.invalidatesStorage())
		m_storage.clear();
	if (sideEffects.invalidatesMemory())
	{
		//cout << "clearing memory" << endl;
		m_memory.clear();
	}
}

void DataFlowAnalyzer::clearKnowledgeIfInvalidated(Expression const& _expr)
{
	SideEffectsCollector sideEffects(m_dialect, _expr, &m_functionSideEffects);
	if (sideEffects.invalidatesStorage())
		m_storage.clear();
	if (sideEffects.invalidatesMemory())
	{
		//cout << "clearing memory" << endl;
		m_memory.clear();
	}
}

void DataFlowAnalyzer::joinKnowledge(
	InvertibleMap<YulString, YulString> const& _olderStorage,
	InvertibleMap<YulString, YulString> const& _olderMemory
)
{
	joinKnowledgeHelper(m_storage, _olderStorage);
	joinKnowledgeHelper(m_memory, _olderMemory);
}

void DataFlowAnalyzer::joinKnowledgeHelper(
	InvertibleMap<YulString, YulString>& _this,
	InvertibleMap<YulString, YulString> const& _older
)
{
	// We clear if the key does not exist in the older map or if the value is different.
	// This also works for memory because _older is an "older version"
	// of m_memory and thus any overlapping write would have cleared the keys
	// that are not known to be different inside m_memory already.
	set<YulString> keysToErase;
	for (auto const& item: _this.values)
	{
		auto it = _older.values.find(item.first);
		if (it == _older.values.end() || it->second != item.second)
			keysToErase.insert(item.first);
	}
	for (auto const& key: keysToErase)
		_this.eraseKey(key);
}

bool DataFlowAnalyzer::inScope(YulString _variableName) const
{
	for (auto const& scope: m_variableScopes | boost::adaptors::reversed)
	{
		if (scope.variables.count(_variableName))
			return true;
		if (scope.isFunction)
			return false;
	}
	return false;
}

boost::optional<pair<YulString, YulString>> DataFlowAnalyzer::isSimpleStore(
	bool _toStorage,
	ExpressionStatement const& _statement
) const
{
	if (_statement.expression.type() == typeid(FunctionCall))
	{
		bool isStore = false;
		FunctionCall const& funCall = boost::get<FunctionCall>(_statement.expression);
		if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
		{
			if (auto const* builtin = dialect->builtin(funCall.functionName.name))
				if (builtin->instruction == (_toStorage ? eth::Instruction::SSTORE : eth::Instruction::MSTORE))
					isStore = true;
		}
		else if (dynamic_cast<WasmDialect const*>(&m_dialect))
			if (m_dialect.builtin(funCall.functionName.name))
			{
				//cout << "wasm call to " << funCall.functionName.name.str() << endl;
				if (funCall.functionName.name == "i64.store"_yulstring && !_toStorage)
					isStore = true;
			}
		//cout << "is store? " << isStore << endl;

		if (
			isStore &&
			funCall.arguments.at(0).type() == typeid(Identifier) &&
			funCall.arguments.at(1).type() == typeid(Identifier)
		)
		{
			YulString key = boost::get<Identifier>(funCall.arguments.at(0)).name;
			YulString value = boost::get<Identifier>(funCall.arguments.at(1)).name;
			//cout << "is store!" << endl;
			return make_pair(key, value);
		}
		//cout << "Nope" << endl;
	}
	return {};
}

