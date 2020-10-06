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
#include <libyul/optimiser/StackToMemoryMover.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AsmData.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{
vector<Statement> generateMemoryStore(
	Dialect const& _dialect,
	langutil::SourceLocation const& _loc,
	YulString _mpos,
	Expression _value
)
{
	BuiltinFunction const* memoryStoreFunction = _dialect.memoryStoreFunction(_dialect.defaultType);
	yulAssert(memoryStoreFunction, "");
	vector<Statement> result;
	result.emplace_back(ExpressionStatement{_loc, FunctionCall{
		_loc,
		Identifier{_loc, memoryStoreFunction->name},
		{
			Literal{_loc, LiteralKind::Number, _mpos, {}},
			std::move(_value)
		}
	}});
	return result;
}
}

void StackToMemoryMover::run(
	OptimiserStepContext& _context,
	u256 _reservedMemory,
	map<YulString, uint64_t> const& _memorySlots,
	uint64_t _numRequiredSlots,
	Block& _block
)
{
	if (!_numRequiredSlots)
	{
		yulAssert(_memorySlots.empty(), "");
		return;
	}

	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackToMemoryMover can only be run on objects using the EVMDialect with object access."
	);
	VariableMemoryOffsetTracker memoryOffsetTracker(_reservedMemory, _memorySlots, _numRequiredSlots);

	ForLoopConditionIntoBody::run(_context, _block);
	ExpressionSplitter::run(_context, _block);

	VariableDeclarationAndAssignmentMover declarationAndAssignmentMover(_context, memoryOffsetTracker);
	declarationAndAssignmentMover(_block);

	ExpressionJoiner::runUntilStabilized(_context, _block);
	ForLoopConditionOutOfBody::run(_context, _block);

	IdentifierMover identifierMover(_context, memoryOffsetTracker);
	identifierMover(_block);
}

void StackToMemoryMover::VariableDeclarationAndAssignmentMover::operator()(FunctionDefinition& _functionDefinition)
{
	for (TypedName const& param: _functionDefinition.parameters + _functionDefinition.returnVariables)
		if (m_memoryOffsetTracker(param.name))
		{
			// TODO: we cannot handle function parameters yet.
			return;
		}
	ASTModifier::operator()(_functionDefinition);
}

void StackToMemoryMover::IdentifierMover::operator()(FunctionDefinition& _functionDefinition)
{
	for (TypedName const& param: _functionDefinition.parameters + _functionDefinition.returnVariables)
		if (m_memoryOffsetTracker(param.name))
		{
			// TODO: we cannot handle function parameters yet.
			return;
		}
	ASTModifier::operator()(_functionDefinition);
}

void StackToMemoryMover::VariableDeclarationAndAssignmentMover::operator()(Block& _block)
{
	using OptionalStatements = std::optional<vector<Statement>>;
	auto containsVariableNeedingEscalation = [&](auto const& _variables) {
		return util::contains_if(_variables, [&](auto const& var) {
			return m_memoryOffsetTracker(var.name);
		});
	};
	auto rewriteAssignmentOrVariableDeclaration = [&](
		langutil::SourceLocation const& _loc,
		auto const& _variables,
		std::unique_ptr<Expression> _value
	) -> std::vector<Statement> {
		VariableDeclaration tempDecl{_loc, {}, std::move(_value)};
		vector<Statement> memoryAssignments;
		vector<Statement> variableAssignments;
		for (auto& var: _variables)
		{
			YulString tempVarName = m_context.dispenser.newName(var.name);
			tempDecl.variables.emplace_back(TypedName{var.location, tempVarName, {}});

			if (optional<YulString> offset = m_memoryOffsetTracker(var.name))
				memoryAssignments += generateMemoryStore(
					m_context.dialect,
					_loc,
					*offset,
					Identifier{_loc, tempVarName}
				);
			else if constexpr (std::is_same_v<std::decay_t<decltype(var)>, Identifier>)
				variableAssignments.emplace_back(Assignment{
					_loc, { Identifier{var.location, var.name} },
					make_unique<Expression>(Identifier{_loc, tempVarName})
				});
			else
				variableAssignments.emplace_back(VariableDeclaration{
					_loc, {std::move(var)},
					make_unique<Expression>(Identifier{_loc, tempVarName})
				});
		}
		std::vector<Statement> result;
		result.emplace_back(std::move(tempDecl));
		std::reverse(memoryAssignments.begin(), memoryAssignments.end());
		result += std::move(memoryAssignments);
		std::reverse(variableAssignments.begin(), variableAssignments.end());
		result += std::move(variableAssignments);
		return result;
	};

	util::iterateReplacing(
		_block.statements,
		[&](Statement& _statement)
		{
			auto defaultVisit = [&]() { ASTModifier::visit(_statement); return OptionalStatements{}; };
			return std::visit(util::GenericVisitor{
				[&](Assignment& _assignment) -> OptionalStatements
				{
					if (!containsVariableNeedingEscalation(_assignment.variableNames))
						return defaultVisit();
					visit(*_assignment.value);
					return {rewriteAssignmentOrVariableDeclaration(
						_assignment.location,
						_assignment.variableNames,
						std::move(_assignment.value)
					)};
				},
				[&](VariableDeclaration& _varDecl) -> OptionalStatements
				{
					if (!containsVariableNeedingEscalation(_varDecl.variables))
						return defaultVisit();
					if (_varDecl.value)
						visit(*_varDecl.value);
					return {rewriteAssignmentOrVariableDeclaration(
						_varDecl.location,
						_varDecl.variables,
						std::move(_varDecl.value)
					)};
				},
				[&](auto&) { return defaultVisit(); }
			}, _statement);
		});
}

void StackToMemoryMover::IdentifierMover::visit(Expression& _expression)
{
	if (Identifier* identifier = std::get_if<Identifier>(&_expression))
		if (optional<YulString> offset = m_memoryOffsetTracker(identifier->name))
		{
			BuiltinFunction const* memoryLoadFunction = m_context.dialect.memoryLoadFunction(m_context.dialect.defaultType);
			yulAssert(memoryLoadFunction, "");
			langutil::SourceLocation loc = identifier->location;
			_expression = FunctionCall{
				loc,
				Identifier{loc, memoryLoadFunction->name}, {
					Literal{
						loc,
						LiteralKind::Number,
						*offset,
						{}
					}
				}
			};
			return;
		}
	ASTModifier::visit(_expression);
}

optional<YulString> StackToMemoryMover::VariableMemoryOffsetTracker::operator()(YulString _variable) const
{
	if (m_memorySlots.count(_variable))
	{
		uint64_t slot = m_memorySlots.at(_variable);
		yulAssert(slot < m_numRequiredSlots, "");
		return YulString{util::toCompactHexWithPrefix(m_reservedMemory + 32 * (m_numRequiredSlots - slot - 1))};
	}
	else
		return std::nullopt;
}
