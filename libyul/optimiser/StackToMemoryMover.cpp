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
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/range/conversion.hpp>

using namespace solidity;
using namespace solidity::yul;

namespace
{
std::vector<Statement> generateMemoryStore(
	Dialect const& _dialect,
	std::shared_ptr<DebugData const> const& _debugData,
	YulString _mpos,
	Expression _value
)
{
	BuiltinFunction const* memoryStoreFunction = _dialect.memoryStoreFunction(_dialect.defaultType);
	yulAssert(memoryStoreFunction, "");
	std::vector<Statement> result;
	result.emplace_back(ExpressionStatement{_debugData, FunctionCall{
		_debugData,
		Identifier{_debugData, memoryStoreFunction->name},
		{
			Literal{_debugData, LiteralKind::Number, _mpos, {}},
			std::move(_value)
		}
	}});
	return result;
}

FunctionCall generateMemoryLoad(Dialect const& _dialect, std::shared_ptr<DebugData const> const& _debugData, YulString _mpos)
{
	BuiltinFunction const* memoryLoadFunction = _dialect.memoryLoadFunction(_dialect.defaultType);
	yulAssert(memoryLoadFunction, "");
	return FunctionCall{
		_debugData,
		Identifier{_debugData, memoryLoadFunction->name}, {
			Literal{
				_debugData,
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
	std::map<YulString, uint64_t> const& _memorySlots,
	uint64_t _numRequiredSlots,
	Block& _block
)
{
	VariableMemoryOffsetTracker memoryOffsetTracker(_reservedMemory, _memorySlots, _numRequiredSlots);
	StackToMemoryMover stackToMemoryMover(
		_context,
		memoryOffsetTracker,
		util::applyMap(
			allFunctionDefinitions(_block),
			util::mapTuple([](YulString _name, FunctionDefinition const* _funDef) {
				return make_pair(_name, _funDef->returnVariables);
			}),
			std::map<YulString, TypedNameList>{}
		)
	);
	stackToMemoryMover(_block);
	_block.statements += std::move(stackToMemoryMover.m_newFunctionDefinitions);
}

StackToMemoryMover::StackToMemoryMover(
	OptimiserStepContext& _context,
	VariableMemoryOffsetTracker const& _memoryOffsetTracker,
	std::map<YulString, TypedNameList> _functionReturnVariables
):
m_context(_context),
m_memoryOffsetTracker(_memoryOffsetTracker),
m_nameDispenser(_context.dispenser),
m_functionReturnVariables(std::move(_functionReturnVariables))
{
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackToMemoryMover can only be run on objects using the EVMDialect with object access."
	);
}

void StackToMemoryMover::operator()(FunctionDefinition& _functionDefinition)
{
	// It is important to first visit the function body, so that it doesn't replace the memory inits for
	// variable arguments we might generate below.
	ASTModifier::operator()(_functionDefinition);

	std::vector<Statement> memoryVariableInits;

	// All function parameters with a memory slot are moved at the beginning of the function body.
	for (TypedName const& param: _functionDefinition.parameters)
		if (auto slot = m_memoryOffsetTracker(param.name))
			memoryVariableInits += generateMemoryStore(
				m_context.dialect,
				param.debugData,
				*slot,
				Identifier{param.debugData, param.name}
			);

	// All memory return variables have to be initialized to zero in memory.
	for (TypedName const& returnVariable: _functionDefinition.returnVariables)
		if (auto slot = m_memoryOffsetTracker(returnVariable.name))
			memoryVariableInits += generateMemoryStore(
				m_context.dialect,
				returnVariable.debugData,
				*slot,
				Literal{returnVariable.debugData, LiteralKind::Number, "0"_yulstring, {}}
			);

	// Special case of a function with a single return argument that needs to move to memory.
	if (_functionDefinition.returnVariables.size() == 1 && m_memoryOffsetTracker(_functionDefinition.returnVariables.front().name))
	{
		TypedNameList stackParameters = _functionDefinition.parameters | ranges::views::filter(
			std::not_fn(m_memoryOffsetTracker)
		) | ranges::to<TypedNameList>;
		// Generate new function without return variable and with only the non-moved parameters.
		YulString newFunctionName = m_context.dispenser.newName(_functionDefinition.name);
		m_newFunctionDefinitions.emplace_back(FunctionDefinition{
			_functionDefinition.debugData,
			newFunctionName,
			stackParameters,
			{},
			std::move(_functionDefinition.body)
		});
		// Generate new names for the arguments to maintain disambiguation.
		std::map<YulString, YulString> newArgumentNames;
		for (TypedName const& _var: stackParameters)
			newArgumentNames[_var.name] = m_context.dispenser.newName(_var.name);
		for (auto& parameter: _functionDefinition.parameters)
			parameter.name = util::valueOrDefault(newArgumentNames, parameter.name, parameter.name);
		// Replace original function by a call to the new function and an assignment to the return variable from memory.
		_functionDefinition.body = Block{_functionDefinition.debugData, std::move(memoryVariableInits)};
		_functionDefinition.body.statements.emplace_back(ExpressionStatement{
			_functionDefinition.debugData,
			FunctionCall{
				_functionDefinition.debugData,
				Identifier{_functionDefinition.debugData, newFunctionName},
				stackParameters | ranges::views::transform([&](TypedName const& _arg) {
					return Expression{Identifier{_arg.debugData, newArgumentNames.at(_arg.name)}};
				}) | ranges::to<std::vector<Expression>>
			}
		});
		_functionDefinition.body.statements.emplace_back(Assignment{
			_functionDefinition.debugData,
			{Identifier{_functionDefinition.debugData, _functionDefinition.returnVariables.front().name}},
			std::make_unique<Expression>(generateMemoryLoad(
				m_context.dialect,
				_functionDefinition.debugData,
				*m_memoryOffsetTracker(_functionDefinition.returnVariables.front().name)
			))
		});
		return;
	}

	if (!memoryVariableInits.empty())
		_functionDefinition.body.statements = std::move(memoryVariableInits) + std::move(_functionDefinition.body.statements);

	_functionDefinition.returnVariables = _functionDefinition.returnVariables | ranges::views::filter(
		std::not_fn(m_memoryOffsetTracker)
	) | ranges::to<TypedNameList>;
}

void StackToMemoryMover::operator()(Block& _block)
{
	using OptionalStatements = std::optional<std::vector<Statement>>;

	auto rewriteAssignmentOrVariableDeclarationLeftHandSide = [this](
		auto& _stmt,
		auto& _lhsVars
	) -> OptionalStatements {
		using StatementType = std::decay_t<decltype(_stmt)>;

		auto debugData = _stmt.debugData;
		if (_lhsVars.size() == 1)
		{
			if (std::optional<YulString> offset = m_memoryOffsetTracker(_lhsVars.front().name))
				return generateMemoryStore(
					m_context.dialect,
					debugData,
					*offset,
					_stmt.value ? *std::move(_stmt.value) : Literal{debugData, LiteralKind::Number, "0"_yulstring, {}}
				);
			else
				return {};
		}
		std::vector<std::optional<YulString>> rhsMemorySlots;
		if (_stmt.value)
		{
			FunctionCall const* functionCall = std::get_if<FunctionCall>(_stmt.value.get());
			yulAssert(functionCall, "");
			if (m_context.dialect.builtin(functionCall->functionName.name))
				rhsMemorySlots = std::vector<std::optional<YulString>>(_lhsVars.size(), std::nullopt);
			else
				rhsMemorySlots =
					m_functionReturnVariables.at(functionCall->functionName.name) |
					ranges::views::transform(m_memoryOffsetTracker) |
					ranges::to<std::vector<std::optional<YulString>>>;
		}
		else
			rhsMemorySlots = std::vector<std::optional<YulString>>(_lhsVars.size(), std::nullopt);

		// Nothing to do, if the right-hand-side remains entirely on the stack and
		// none of the variables in the left-hand-side are moved.
		if (
			ranges::none_of(rhsMemorySlots, [](std::optional<YulString> const& _slot) { return _slot.has_value(); }) &&
			!util::contains_if(_lhsVars, m_memoryOffsetTracker)
		)
			return {};

		std::vector<Statement> memoryAssignments;
		std::vector<Statement> variableAssignments;
		VariableDeclaration tempDecl{debugData, {}, std::move(_stmt.value)};

		yulAssert(rhsMemorySlots.size() == _lhsVars.size(), "");
		for (auto&& [lhsVar, rhsSlot]: ranges::views::zip(_lhsVars, rhsMemorySlots))
		{
			std::unique_ptr<Expression> rhs;
			if (rhsSlot)
				rhs = std::make_unique<Expression>(generateMemoryLoad(m_context.dialect, debugData, *rhsSlot));
			else
			{
				YulString tempVarName = m_nameDispenser.newName(lhsVar.name);
				tempDecl.variables.emplace_back(TypedName{lhsVar.debugData, tempVarName, {}});
				rhs = std::make_unique<Expression>(Identifier{debugData, tempVarName});
			}

			if (std::optional<YulString> offset = m_memoryOffsetTracker(lhsVar.name))
				memoryAssignments += generateMemoryStore(
					m_context.dialect,
					_stmt.debugData,
					*offset,
					std::move(*rhs)
				);
			else
				variableAssignments.emplace_back(StatementType{
					debugData,
					{ std::move(lhsVar) },
					std::move(rhs)
				});
		}

		std::vector<Statement> result;
		if (tempDecl.variables.empty())
			result.emplace_back(ExpressionStatement{debugData, *std::move(tempDecl.value)});
		else
			result.emplace_back(std::move(tempDecl));
		reverse(memoryAssignments.begin(), memoryAssignments.end());
		result += std::move(memoryAssignments);
		reverse(variableAssignments.begin(), variableAssignments.end());
		result += std::move(variableAssignments);
		return OptionalStatements{std::move(result)};
	};

	util::iterateReplacing(
		_block.statements,
		[&](Statement& _statement) -> OptionalStatements
		{
			visit(_statement);
			if (auto* assignment = std::get_if<Assignment>(&_statement))
				return rewriteAssignmentOrVariableDeclarationLeftHandSide(*assignment, assignment->variableNames);
			else if (auto* varDecl = std::get_if<VariableDeclaration>(&_statement))
				return rewriteAssignmentOrVariableDeclarationLeftHandSide(*varDecl, varDecl->variables);
			return {};
		}
	);
}

void StackToMemoryMover::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);
	if (Identifier* identifier = std::get_if<Identifier>(&_expression))
		if (std::optional<YulString> offset = m_memoryOffsetTracker(identifier->name))
			_expression = generateMemoryLoad(m_context.dialect, identifier->debugData, *offset);
}

std::optional<YulString> StackToMemoryMover::VariableMemoryOffsetTracker::operator()(YulString _variable) const
{
	if (m_memorySlots.count(_variable))
	{
		uint64_t slot = m_memorySlots.at(_variable);
		yulAssert(slot < m_numRequiredSlots, "");
		return YulString{toCompactHexWithPrefix(m_reservedMemory + 32 * (m_numRequiredSlots - slot - 1))};
	}
	else
		return std::nullopt;
}

std::optional<YulString> StackToMemoryMover::VariableMemoryOffsetTracker::operator()(TypedName const& _variable) const
{
	return (*this)(_variable.name);
}

std::optional<YulString> StackToMemoryMover::VariableMemoryOffsetTracker::operator()(Identifier const& _variable) const
{
	return (*this)(_variable.name);
}
