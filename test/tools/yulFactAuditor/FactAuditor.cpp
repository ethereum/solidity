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

#include <test/tools/yulFactAuditor/FactAuditor.h>

#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/optimiser/Semantics.h>

#include <memory>
#include <set>
#include <string_view>
#include <utility>
#include <variant>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

namespace
{

/// Collects the variables passed to storage, memory and keccak256 builtins; pair facts are only recorded among them.
class AccessArgumentCollector: public ASTWalker
{
public:
	explicit AccessArgumentCollector(Dialect const& _dialect): m_dialect(_dialect) {}
	using ASTWalker::operator();
	void operator()(FunctionCall const& _call) override
	{
		static std::set<std::string_view> const accessBuiltins{
			"sload", "sstore", "mload", "mstore", "mstore8", "tload", "tstore", "keccak256"
		};
		if (accessBuiltins.contains(resolveFunctionName(_call.functionName, m_dialect)))
			for (auto const& argument: _call.arguments)
				if (auto const* identifier = std::get_if<Identifier>(&argument))
					accessArguments.insert(identifier->name);
		ASTWalker::operator()(_call);
	}
	std::set<YulName> accessArguments;

private:
	Dialect const& m_dialect;
};

/// A DataFlowAnalyzer that records, for each statement, the facts it claims right before it.
class FactRecorder: public DataFlowAnalyzer
{
public:
	FactRecorder(Dialect const& _dialect, Block const& _ast, std::set<YulName> _accessArguments):
		DataFlowAnalyzer(
			_dialect,
			MemoryAndStorage::Analyze,
			SideEffectsPropagator::sideEffects(_dialect, CallGraphGenerator::callGraph(_ast))
		),
		m_accessArguments(std::move(_accessArguments))
	{}
	using DataFlowAnalyzer::operator();
	using ASTModifier::visit;
	void visit(Statement& _statement) override
	{
		// Function definitions never execute, so there is nothing to check before them.
		if (!std::holds_alternative<FunctionDefinition>(_statement))
		{
			recordCurrentFacts();
			if (!m_checks.empty())
				checksBefore[&_statement] = std::exchange(m_checks, {});
		}
		DataFlowAnalyzer::visit(_statement);
	}

	std::vector<std::string> facts;
	/// The checks of the facts claimed before each statement, for statements that have any.
	std::map<Statement const*, std::vector<Statement>> checksBefore;

private:
	void record(Expression _fact)
	{
		facts.push_back(std::visit(AsmPrinter{m_dialect}, _fact));
		Block onViolation;
		onViolation.statements.emplace_back(
			ExpressionStatement{{}, call("sstore", number(factViolationSlot), number(facts.size()))}
		);
		onViolation.statements.emplace_back(ExpressionStatement{{}, call("stop")});
		m_checks.emplace_back(
			If{{}, std::make_unique<Expression>(call("iszero", std::move(_fact))), std::move(onViolation)}
		);
	}

	void recordCurrentFacts()
	{
		// A copy: queries update the KnowledgeBase cache, which would mask the stale state being tested for.
		KnowledgeBase knowledge = m_knowledgeBase;
		std::vector<YulName> accessed;
		for (YulName const& variable: variablesInScope())
		{
			recordVariableFacts(knowledge, variable);
			if (m_accessArguments.contains(variable))
				accessed.push_back(variable);
		}
		for (YulName const& a: accessed)
			for (YulName const& b: accessed)
				recordPairFacts(knowledge, a, b);
	}

	void recordVariableFacts(KnowledgeBase& _knowledge, YulName const& _variable)
	{
		// A fact mentioning an out-of-scope variable cannot be written as a check.
		if (auto storedValue = storageValue(_variable); storedValue && inScope(*storedValue))
			record(call("eq", call("sload", identifier(_variable)), identifier(*storedValue)));
		if (auto loadedValue = memoryValue(_variable); loadedValue && inScope(*loadedValue))
			record(call("eq", call("mload", identifier(_variable)), identifier(*loadedValue)));
		if (auto const* assigned = variableValue(_variable); assigned && assigned->value && allReferencesInScope(_variable))
			record(call("eq", identifier(_variable), ASTCopier{}.translate(*assigned->value)));
		if (auto constant = _knowledge.valueIfKnownConstant(_variable))
			record(call("eq", identifier(_variable), number(*constant)));
	}

	void recordPairFacts(KnowledgeBase& _knowledge, YulName const& _a, YulName const& _b)
	{
		if (auto hash = keccakValue(_a, _b); hash && inScope(*hash))
			record(call("eq", call("keccak256", identifier(_a), identifier(_b)), identifier(*hash)));
		if (_a == _b)
			return;
		if (auto difference = _knowledge.differenceIfKnownConstant(_a, _b))
			record(call("eq", call("sub", identifier(_a), identifier(_b)), number(*difference)));
		// The remaining facts are symmetric.
		if (_b < _a)
			return;
		if (_knowledge.knownToBeDifferent(_a, _b))
			record(call("iszero", call("eq", identifier(_a), identifier(_b))));
		if (_knowledge.knownToBeDifferentByAtLeast32(_a, _b))
			record(call(
				"and",
				call("gt", call("sub", identifier(_a), identifier(_b)), number(31)),
				call("gt", call("sub", identifier(_b), identifier(_a)), number(31))
			));
	}

	template<typename... Arguments>
	Expression call(std::string_view const _builtin, Arguments&&... _arguments) const
	{
		std::optional<BuiltinHandle> const handle = m_dialect.findBuiltin(_builtin);
		yulAssert(handle, "builtin not in the dialect");
		std::vector<Expression> arguments;
		(arguments.emplace_back(std::forward<Arguments>(_arguments)), ...);
		return FunctionCall{{}, BuiltinName{{}, *handle}, std::move(arguments)};
	}

	static Expression identifier(YulName const _name) { return Identifier{{}, _name}; }
	static Expression number(u256 const& _value)
	{
		return Literal{{}, LiteralKind::Number, LiteralValue{_value, formatNumber(_value)}};
	}

	std::set<YulName> variablesInScope() const
	{
		std::set<YulName> variables;
		for (auto const& scope: m_variableScopes)
			for (auto const& variable: scope.variables)
				if (inScope(variable))
					variables.insert(variable);
		return variables;
	}

	bool allReferencesInScope(YulName const& _variable) const
	{
		if (auto const* references = sortedReferences(_variable))
			for (auto const& reference: *references)
				if (!inScope(reference))
					return false;
		return true;
	}

	std::set<YulName> m_accessArguments;
	std::vector<Statement> m_checks;
};

class CheckInserter: public ASTModifier
{
public:
	explicit CheckInserter(std::map<Statement const*, std::vector<Statement>>& _checksBefore):
		m_checksBefore(_checksBefore)
	{}
	using ASTModifier::operator();
	void operator()(Block& _block) override
	{
		ASTModifier::operator()(_block);
		std::vector<Statement> statements;
		for (auto& statement: _block.statements)
		{
			if (auto const it = m_checksBefore.find(&statement); it != m_checksBefore.end())
				for (auto& check: it->second)
					statements.emplace_back(std::move(check));
			statements.emplace_back(std::move(statement));
		}
		_block.statements = std::move(statements);
	}

private:
	std::map<Statement const*, std::vector<Statement>>& m_checksBefore;
};

}

std::vector<std::string> solidity::yul::test::instrumentWithFactChecks(Dialect const& _dialect, Block& _ast)
{
	AccessArgumentCollector collector{_dialect};
	collector(_ast);
	FactRecorder recorder{_dialect, _ast, std::move(collector.accessArguments)};
	recorder(_ast);
	CheckInserter{recorder.checksBefore}(_ast);
	return std::move(recorder.facts);
}

std::optional<std::size_t> solidity::yul::test::violatedFactIndex(std::map<util::h256, util::h256> const& _storage)
{
	auto const entry = _storage.find(util::h256(factViolationSlot));
	if (entry == _storage.end() || entry->second == util::h256())
		return std::nullopt;
	return static_cast<std::size_t>(u256(entry->second)) - 1;
}
