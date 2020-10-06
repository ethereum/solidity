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
#include <libyul/optimiser/FunctionDefinitionCollector.h>
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
FunctionCall generateMemoryLoad(Dialect const& _dialect, langutil::SourceLocation const& _loc, YulString _mpos)
{
	BuiltinFunction const* memoryLoadFunction = _dialect.memoryLoadFunction(_dialect.defaultType);
	yulAssert(memoryLoadFunction, "");
	return FunctionCall{
		_loc,
		Identifier{_loc, memoryLoadFunction->name}, {
			Literal{
				_loc,
				LiteralKind::Number,
				_mpos,
				{}
			}
		}
	};
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

	FunctionCallRewriter functionCallRewriter(_context, memoryOffsetTracker, FunctionDefinitionCollector::run(_block));
	functionCallRewriter(_block);

	VariableDeclarationAndAssignmentMover declarationAndAssignmentMover(_context, memoryOffsetTracker);
	declarationAndAssignmentMover(_block);

	ExpressionJoiner::runUntilStabilized(_context, _block);
	ForLoopConditionOutOfBody::run(_context, _block);

	FunctionDefinitionRewriter functionDefinitionRewriter(_context, memoryOffsetTracker);
	functionDefinitionRewriter(_block);

	IdentifierMover identifierMover(_context, memoryOffsetTracker);
	identifierMover(_block);
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
			_expression = generateMemoryLoad(m_context.dialect, identifier->location, *offset);
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

void StackToMemoryMover::FunctionDefinitionRewriter::operator()(FunctionDefinition& _functionDefinition)
{
	vector<Statement> returnVariableMemoryInits;
	TypedNameList parameters;
	for (TypedName& parameter: _functionDefinition.parameters)
		if (!m_memoryOffsetTracker(parameter.name))
			parameters.emplace_back(std::move(parameter));
	_functionDefinition.parameters = std::move(parameters);
	TypedNameList returnVariables;
	for (TypedName& returnVariable: _functionDefinition.returnVariables)
		if (auto slot = m_memoryOffsetTracker(returnVariable.name))
			returnVariableMemoryInits += generateMemoryStore(
				m_context.dialect,
				returnVariable.location,
				*slot,
				Literal{returnVariable.location, LiteralKind::Number, "0"_yulstring, {}}
			);
		else
			returnVariables.emplace_back(std::move(returnVariable));
	_functionDefinition.returnVariables = std::move(returnVariables);

	if (!returnVariableMemoryInits.empty())
	{
		returnVariableMemoryInits += std::move(_functionDefinition.body.statements);
		_functionDefinition.body.statements = std::move(returnVariableMemoryInits);
	}
}

StackToMemoryMover::FunctionCallRewriter::FunctionCallRewriter(
	OptimiserStepContext& _context,
	VariableMemoryOffsetTracker const& _memoryOffsetTracker,
	std::map<YulString, FunctionDefinition const*> const& _functionDefinitions
): m_context(_context), m_memoryOffsetTracker(_memoryOffsetTracker)
{
	for (auto [functionName, functionDefinition]: _functionDefinitions)
	{
		FunctionArguments& functionArguments = m_functionArguments[functionName];
		for (TypedName const& parameter: functionDefinition->parameters)
			functionArguments.parameters.push_back(parameter.name);
		for (TypedName const& returnVariable: functionDefinition->returnVariables)
			functionArguments.returnVariables.push_back(returnVariable.name);
	}
}

void StackToMemoryMover::FunctionCallRewriter::operator()(FunctionCall& _functionCall)
{
	if (!m_functionArguments.count(_functionCall.functionName.name))
	{
		// If it is not in the list of user-defined functions, it should be a builtin.
		yulAssert(m_context.dialect.builtin(_functionCall.functionName.name), "");
		ASTModifier::operator()(_functionCall);
		return;
	}

	yulAssert(m_slotsForCurrentReturns.empty(), "");

	FunctionArguments const& functionArguments = m_functionArguments.at(_functionCall.functionName.name);

	yulAssert(_functionCall.arguments.size() == functionArguments.parameters.size(), "");

	vector<Expression> arguments;
	for (size_t i = 0; i < functionArguments.parameters.size(); ++i)
	{
		yulAssert(
			holds_alternative<Identifier>(_functionCall.arguments[i]) ||
			holds_alternative<Literal>(_functionCall.arguments[i]),
			"Expected fully split expressions."
		);

		if (auto slot = m_memoryOffsetTracker(functionArguments.parameters[i]))
		{
			auto loc = locationOf(_functionCall.arguments[i]);
			m_statementsToPrefix += generateMemoryStore(
				m_context.dialect,
				loc,
				*slot,
				std::move(_functionCall.arguments[i])
			);
		}
		else
			arguments.emplace_back(_functionCall.arguments[i]);
	}

	for (YulString returnVariable: functionArguments.returnVariables)
		if (auto slot = m_memoryOffsetTracker(returnVariable))
			m_slotsForCurrentReturns.emplace_back(*slot);
		else
			m_slotsForCurrentReturns.emplace_back(std::nullopt);

	_functionCall.arguments = std::move(arguments);
}

void StackToMemoryMover::FunctionCallRewriter::operator()(Block& _block)
{
	util::iterateReplacing(_block.statements, [&](Statement& _statement) -> std::optional<vector<Statement>> {
		yulAssert(m_statementsToPrefix.empty(), "");
		vector<Statement> statementsToSuffix;
		auto handleVariableDeclarationOrAssignment = [&](auto& _stmt, auto& _variableList) {
			visit(*_stmt.value);
			if (!m_slotsForCurrentReturns.empty())
			{
				std::decay_t<decltype(_variableList)> variableList;
				yulAssert(_variableList.size() == m_slotsForCurrentReturns.size(), "");
				for (size_t i = 0; i < m_slotsForCurrentReturns.size(); ++i)
					if (auto slot = m_slotsForCurrentReturns[i])
					{
						auto loc = _variableList[i].location;
						statementsToSuffix.emplace_back(std::decay_t<decltype(_stmt)>{
							loc,
							{std::move(_variableList[i])},
							std::make_unique<Expression>(generateMemoryLoad(
								m_context.dialect,
								loc,
								*slot
							))
						});
					}
					else
						variableList.emplace_back(move(_variableList[i]));
				if (variableList.empty())
					_statement = ExpressionStatement{_stmt.location, std::move(*_stmt.value)};
				else
					_variableList = move(variableList);
				m_slotsForCurrentReturns.clear();
			}
		};
		if (Assignment* _assignment = std::get_if<Assignment>(&_statement))
			handleVariableDeclarationOrAssignment(*_assignment, _assignment->variableNames);
		else if (VariableDeclaration* _variableDeclaration = std::get_if<VariableDeclaration>(&_statement))
		{
			if (_variableDeclaration->value)
				handleVariableDeclarationOrAssignment(*_variableDeclaration, _variableDeclaration->variables);
		}
		else
			visit(_statement);

		if (m_statementsToPrefix.empty() && statementsToSuffix.empty())
			return {};
		vector<Statement> result;
		result.swap(m_statementsToPrefix);
		result.emplace_back(std::move(_statement));
		result += std::move(statementsToSuffix);
		statementsToSuffix.clear();
		return optional<vector<Statement>>(move(result));
	});
}
