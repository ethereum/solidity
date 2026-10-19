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

#include <test/tools/yulInterpreter/SideEffectsAuditor.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/AST.h>
#include <libyul/AsmPrinter.h>
#include <libyul/ControlFlowSideEffects.h>
#include <libyul/ControlFlowSideEffectsCollector.h>
#include <libyul/Dialect.h>
#include <libyul/SideEffects.h>
#include <libyul/Utilities.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/Semantics.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/Common.h>

#include <fmt/format.h>

#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

namespace
{

class FlowCollector: public ASTWalker
{
public:
	FlowCollector(Dialect const& _dialect, std::map<YulName, ControlFlowSideEffects> const& _functionFlows):
		m_dialect(_dialect), m_functionFlows(_functionFlows)
	{}
	using ASTWalker::operator();
	void operator()(FunctionCall const& _call) override
	{
		ASTWalker::operator()(_call);
		BuiltinFunction const* builtin = resolveBuiltinFunction(_call.functionName, m_dialect);
		involvesUserFunction = involvesUserFunction || !builtin;
		ControlFlowSideEffects const& callFlow =
			builtin ?
			builtin->controlFlowSideEffects :
			m_functionFlows.at(std::get<Identifier>(_call.functionName).name);
		flow.canTerminate = flow.canTerminate || callFlow.canTerminate;
		flow.canRevert = flow.canRevert || callFlow.canRevert;
		flow.canContinue = flow.canContinue && callFlow.canContinue;
	}

	ControlFlowSideEffects flow;
	bool involvesUserFunction = false;

private:
	Dialect const& m_dialect;
	std::map<YulName, ControlFlowSideEffects> const& m_functionFlows;
};

struct Claims
{
	SideEffects effects;
	ControlFlowSideEffects flow;
	bool involvesUserFunction;
};

enum class End { Continued, Reverted, Terminated };

/// The claims about the calls of a program and the outcome of checking them, shared by all interpreters of a run.
class Auditor
{
public:
	Auditor(Dialect const& _dialect, Block const& _ast):
		m_dialect(_dialect),
		m_functionEffects(SideEffectsPropagator::sideEffects(_dialect, CallGraphGenerator::callGraph(_ast))),
		m_functionFlows(ControlFlowSideEffectsCollector{_dialect, _ast}.functionSideEffectsNamed())
	{}

	Claims const& claims(FunctionCall const& _call)
	{
		auto it = m_claims.find(&_call);
		if (it == m_claims.end())
		{
			SideEffectsCollector effects{m_dialect, &m_functionEffects};
			effects(_call);
			FlowCollector flow{m_dialect, m_functionFlows};
			flow(_call);
			it = m_claims.emplace(&_call, Claims{effects.sideEffects(), flow.flow, flow.involvesUserFunction}).first;
		}
		return it->second;
	}

	void recordClaimCheck(Claim const _claim, bool const _holds, Claims const& _claims, FunctionCall const& _call)
	{
		++audit.checks[_claim];
		if (_claims.involvesUserFunction)
			++audit.checksInvolvingUserFunctions[_claim];
		if (!_holds && !audit.violation)
			audit.violation = fmt::format("\"{}\" is claimed about {}", _claim, AsmPrinter{m_dialect}(_call));
	}

	void checkEnd(Claims const& _claims, End const _end, FunctionCall const& _call)
	{
		if (!_claims.flow.canContinue)
			recordClaimCheck(Claim::CannotContinue, _end != End::Continued, _claims, _call);
		if (!_claims.flow.canRevert)
			recordClaimCheck(Claim::CannotRevert, _end != End::Reverted, _claims, _call);
		if (!_claims.flow.canTerminate)
			recordClaimCheck(Claim::CannotTerminate, _end != End::Terminated, _claims, _call);
		if (_end == End::Continued)
			return;
		// The claims below are checked in full once the call has continued.
		if (removable(_claims))
			recordClaimCheck(Claim::CanBeRemoved, false, _claims, _call);
		if (_claims.effects.movable)
			recordClaimCheck(Claim::Movable, false, _claims, _call);
	}

	static bool removable(Claims const& _claims)
	{
		return _claims.effects.canBeRemoved || _claims.effects.canBeRemovedIfNoMSize;
	}

	SideEffectsAudit audit;

private:
	Dialect const& m_dialect;
	std::map<FunctionHandle, SideEffects> m_functionEffects;
	std::map<YulName, ControlFlowSideEffects> m_functionFlows;
	std::map<FunctionCall const*, Claims> m_claims;
};

/// A copy of @a _contents if checking the claim @a _effect about a call needs it, i.e. if the claim allows the call
/// to read but not to write, or if the call is claimed to be removable despite being allowed to write.
/// Otherwise, whether the contents changed follows from whether the call accessed them.
template <typename Map>
std::optional<Map> snapshotIfNeeded(Map const& _contents, SideEffects::Effect const _effect, bool const _removable)
{
	if (_effect == SideEffects::Read || (_effect == SideEffects::Write && _removable))
		return _contents;
	return std::nullopt;
}

/// @returns true if the maps agree on every key. A missing key counts as zero: loads insert the keys they read.
template <typename Map>
bool sameContents(Map const& _a, Map const& _b)
{
	auto const covers = [](Map const& _covering, Map const& _covered) {
		for (auto const& [key, value]: _covered)
			if (value != typename Map::mapped_type{})
			{
				auto const entry = _covering.find(key);
				if (entry == _covering.end() || entry->second != value)
					return false;
			}
		return true;
	};
	return covers(_a, _b) && covers(_b, _a);
}

class AuditingInterpreter: public Interpreter
{
public:
	AuditingInterpreter(
		Auditor& _auditor,
		InterpreterState& _state,
		Dialect const& _dialect,
		test::Scope& _scope,
		bool _disableExternalCalls,
		bool _disableMemoryTracing,
		std::map<YulName, u256> _variables = {}
	):
		Interpreter(_state, _dialect, _scope, _disableExternalCalls, _disableMemoryTracing, std::move(_variables)),
		m_auditor(_auditor)
	{}

protected:
	u256 evaluate(Expression const& _expression) override;
	std::vector<u256> evaluateMulti(Expression const& _expression) override;

private:
	Auditor& m_auditor;
};

class AuditingExpressionEvaluator: public ExpressionEvaluator
{
public:
	AuditingExpressionEvaluator(
		Auditor& _auditor,
		InterpreterState& _state,
		Dialect const& _dialect,
		test::Scope& _scope,
		std::map<YulName, u256> const& _variables,
		bool _disableExternalCalls,
		bool _disableMemoryTrace
	):
		ExpressionEvaluator(_state, _dialect, _scope, _variables, _disableExternalCalls, _disableMemoryTrace),
		m_auditor(_auditor)
	{}

	using ExpressionEvaluator::operator();
	void operator()(FunctionCall const& _call) override
	{
		Claims const& claims = m_auditor.claims(_call);
		bool const removable = Auditor::removable(claims);
		auto const storageBefore = snapshotIfNeeded(m_state.storage, claims.effects.storage, removable);
		auto const memoryBefore = snapshotIfNeeded(m_state.memory, claims.effects.memory, removable);
		std::size_t const storageAccessesBefore = m_state.numStorageAccesses;
		std::size_t const memoryAccessesBefore = m_state.numMemoryAccesses;
		try
		{
			ExpressionEvaluator::operator()(_call);
		}
		catch (ExplicitlyTerminatedWithFailure const&)
		{
			m_auditor.checkEnd(claims, End::Reverted, _call);
			throw;
		}
		catch (ExplicitlyTerminated const&)
		{
			m_auditor.checkEnd(claims, End::Terminated, _call);
			throw;
		}
		m_auditor.checkEnd(claims, End::Continued, _call);

		bool const storageAccessed = m_state.numStorageAccesses != storageAccessesBefore;
		bool const memoryAccessed = m_state.numMemoryAccesses != memoryAccessesBefore;
		bool const storageUnchanged =
			storageBefore ? sameContents(*storageBefore, m_state.storage) : !storageAccessed;
		bool const memoryUnchanged =
			memoryBefore ? sameContents(*memoryBefore, m_state.memory) : !memoryAccessed;
		// The access claims go first: without a snapshot, an access also fails the write claim.
		if (claims.effects.storage == SideEffects::None)
			m_auditor.recordClaimCheck(Claim::DoesNotAccessStorage, !storageAccessed, claims, _call);
		if (claims.effects.storage != SideEffects::Write)
			m_auditor.recordClaimCheck(Claim::DoesNotWriteStorage, storageUnchanged, claims, _call);
		if (claims.effects.memory == SideEffects::None)
			m_auditor.recordClaimCheck(Claim::DoesNotAccessMemory, !memoryAccessed, claims, _call);
		if (claims.effects.memory != SideEffects::Write)
			m_auditor.recordClaimCheck(Claim::DoesNotWriteMemory, memoryUnchanged, claims, _call);
		if (removable)
			m_auditor.recordClaimCheck(Claim::CanBeRemoved, storageUnchanged && memoryUnchanged, claims, _call);
		if (claims.effects.movable)
		{
			// The second evaluation is not audited.
			ExpressionEvaluator again(
				m_state, m_dialect, m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace
			);
			again(_call);
			bool const sameResult = !storageAccessed && !memoryAccessed && again.values() == values();
			m_auditor.recordClaimCheck(Claim::Movable, sameResult, claims, _call);
		}
	}

protected:
	std::unique_ptr<Interpreter> makeInterpreterCopy(std::map<YulName, u256> _variables = {}) const override
	{
		return std::make_unique<AuditingInterpreter>(
			m_auditor,
			m_state,
			m_dialect,
			m_scope,
			m_disableExternalCalls,
			m_disableMemoryTrace,
			std::move(_variables)
		);
	}

private:
	Auditor& m_auditor;
};

u256 AuditingInterpreter::evaluate(Expression const& _expression)
{
	AuditingExpressionEvaluator evaluator(
		m_auditor, m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace
	);
	evaluator.visit(_expression);
	return evaluator.value();
}

std::vector<u256> AuditingInterpreter::evaluateMulti(Expression const& _expression)
{
	AuditingExpressionEvaluator evaluator(
		m_auditor, m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace
	);
	evaluator.visit(_expression);
	return evaluator.values();
}

}

std::string_view solidity::yul::test::format_as(Claim const _claim)
{
	switch (_claim)
	{
	case Claim::CannotContinue:
		return "cannot continue";
	case Claim::CannotRevert:
		return "cannot revert";
	case Claim::CannotTerminate:
		return "cannot terminate";
	case Claim::DoesNotWriteStorage:
		return "does not write storage";
	case Claim::DoesNotAccessStorage:
		return "does not access storage";
	case Claim::DoesNotWriteMemory:
		return "does not write memory";
	case Claim::DoesNotAccessMemory:
		return "does not access memory";
	case Claim::CanBeRemoved:
		return "can be removed";
	case Claim::Movable:
		return "movable";
	}
	util::unreachable();
}

SideEffectsAudit solidity::yul::test::auditSideEffects(
	InterpreterState& _state,
	Dialect const& _dialect,
	Block const& _ast
)
{
	Auditor auditor{_dialect, _ast};
	test::Scope scope;
	try
	{
		// Calls to other contracts would be run by an interpreter that does not check anything.
		AuditingInterpreter{
			auditor, _state, _dialect, scope, /*disableExternalCalls=*/true, /*disableMemoryTracing=*/true
		}(_ast);
	}
	catch (ExplicitlyTerminated const&)
	{
	}
	return std::move(auditor.audit);
}
