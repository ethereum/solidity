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
#include <libyul/optimiser/FunctionDefinitionCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>

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
	VariableMemoryOffsetTracker memoryOffsetTracker(_reservedMemory, _memorySlots, _numRequiredSlots);
	StackToMemoryMover stackToMemoryMover(
		_context,
		memoryOffsetTracker,
		util::applyMap(
			FunctionDefinitionCollector::run(_block),
			util::mapTuple([](YulString _name, FunctionDefinition const* _funDef) {
				return std::make_pair(_name, _funDef->returnVariables);
			}),
			map<YulString, TypedNameList>{}
		)
	);
	stackToMemoryMover(_block);
	_block.statements += move(stackToMemoryMover.m_newFunctionDefinitions);
}

StackToMemoryMover::StackToMemoryMover(
	OptimiserStepContext& _context,
	VariableMemoryOffsetTracker const& _memoryOffsetTracker,
	map<YulString, TypedNameList> _functionReturnVariables
):
m_context(_context),
m_memoryOffsetTracker(_memoryOffsetTracker),
m_nameDispenser(_context.dispenser),
m_functionReturnVariables(move(_functionReturnVariables))
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

	vector<Statement> memoryVariableInits;

	// All function arguments with a memory slot are moved at the beginning of the function body.
	for (TypedName const& param: _functionDefinition.parameters)
		if (auto slot = m_memoryOffsetTracker(param.name))
			memoryVariableInits += generateMemoryStore(
				m_context.dialect,
				param.location,
				*slot,
				Identifier{param.location, param.name}
			);

	// All memory return variables have to be initialized to zero in memory.
	for (TypedName const& returnVariable: _functionDefinition.returnVariables)
		if (auto slot = m_memoryOffsetTracker(returnVariable.name))
			memoryVariableInits += generateMemoryStore(
				m_context.dialect,
				returnVariable.location,
				*slot,
				Literal{returnVariable.location, LiteralKind::Number, "0"_yulstring, {}}
			);

	// Special case of a function with a single return argument that needs to move to memory.
	if (_functionDefinition.returnVariables.size() == 1 && m_memoryOffsetTracker(_functionDefinition.returnVariables.front().name))
	{
		TypedNameList stackArguments = _functionDefinition.parameters | ranges::views::filter([&](TypedName const& _arg){
			return !m_memoryOffsetTracker(_arg.name);
		}) | ranges::to<TypedNameList>;
		// Generate new function without return argument and with only the non-moved arguments.
		YulString newFunctionName = m_context.dispenser.newName(_functionDefinition.name);
		m_newFunctionDefinitions.emplace_back(FunctionDefinition{
			_functionDefinition.location,
			newFunctionName,
			stackArguments,
			{},
			move(_functionDefinition.body)
		});
		// Replace original function by a call to the new function and an assignment to the return variable from memory.
		_functionDefinition.body = Block{_functionDefinition.location, move(memoryVariableInits)};
		_functionDefinition.body.statements.emplace_back(ExpressionStatement{
			_functionDefinition.location,
			FunctionCall{
				_functionDefinition.location,
				Identifier{_functionDefinition.location, newFunctionName},
				stackArguments | ranges::views::transform([&](TypedName const& _arg) {
					return Expression{Identifier{
						_functionDefinition.location,
						_arg.name
					}};
				}) | ranges::to<vector<Expression>>
			}
		});
		_functionDefinition.body.statements.emplace_back(Assignment{
			_functionDefinition.location,
			{Identifier{_functionDefinition.location, _functionDefinition.returnVariables.front().name}},
			make_unique<Expression>(generateMemoryLoad(
				m_context.dialect,
				_functionDefinition.location,
				*m_memoryOffsetTracker(_functionDefinition.returnVariables.front().name)
			))
		});
		return;
	}

	if (!memoryVariableInits.empty())
	{
		memoryVariableInits += move(_functionDefinition.body.statements);
		_functionDefinition.body.statements = move(memoryVariableInits);
	}

	_functionDefinition.returnVariables = _functionDefinition.returnVariables | ranges::views::filter([&](TypedName const& _name){
		return !m_memoryOffsetTracker(_name.name);
	}) | ranges::to<TypedNameList>;
}

void StackToMemoryMover::operator()(Block& _block)
{
	using OptionalStatements = std::optional<vector<Statement>>;

	auto rewriteAssignmentOrVariableDeclarationLeftHandSide = [this](
		auto& _stmt,
		auto& _lhsVars
	) -> OptionalStatements {
		using StatementType = decay_t<decltype(_stmt)>;

		langutil::SourceLocation  loc = _stmt.location;
		if (_lhsVars.size() == 1)
		{
			optional<YulString> offset = m_memoryOffsetTracker(_lhsVars.front().name);
			if (offset)
				return generateMemoryStore(
					m_context.dialect,
					loc,
					*offset,
					_stmt.value ? *std::move(_stmt.value) : Literal{loc, LiteralKind::Number, "0"_yulstring, {}}
				);
			else
				return {};
		}
		FunctionCall const* functionCall = get_if<FunctionCall>(_stmt.value.get());
		yulAssert(functionCall, "");
		auto rhsSlots = m_functionReturnVariables.at(functionCall->functionName.name) |
			ranges::views::transform([&](auto const& _var) {
				return m_memoryOffsetTracker(_var.name);
			}) | ranges::to<vector<optional<YulString>>>;

		// Nothing to do, if the right-hand-side remains entirely on the stack and
		// none of the variables in the left-hand-side are moved.
		if (
			ranges::none_of(rhsSlots, [](optional<YulString> const& _slot) { return _slot.has_value(); }) &&
			!util::contains_if(_lhsVars, [&](auto const& var) { return m_memoryOffsetTracker(var.name); })
		)
			return {};

		vector<Statement> memoryAssignments;
		vector<Statement> variableAssignments;
		VariableDeclaration tempDecl{loc, {}, std::move(_stmt.value)};

		yulAssert(rhsSlots.size() == _lhsVars.size(), "");
		for (auto&& [lhsVar, rhsSlot]: ranges::views::zip(_lhsVars, rhsSlots))
		{
			unique_ptr<Expression> rhs;
			if (rhsSlot)
				rhs = make_unique<Expression>(generateMemoryLoad(m_context.dialect, loc, *rhsSlot));
			else
			{
				YulString tempVarName = m_nameDispenser.newName(lhsVar.name);
				tempDecl.variables.emplace_back(TypedName{lhsVar.location, tempVarName, {}});
				rhs = make_unique<Expression>(Identifier{loc, tempVarName});
			}

			if (optional<YulString> offset = m_memoryOffsetTracker(lhsVar.name))
				memoryAssignments += generateMemoryStore(
					m_context.dialect,
					loc,
					*offset,
					move(*rhs)
				);
			else
				variableAssignments.emplace_back(StatementType{
					loc, { std::move(lhsVar) },
					move(rhs)
				});
		}

		std::vector<Statement> result;
		if (tempDecl.variables.empty())
			result.emplace_back(ExpressionStatement{loc, *move(tempDecl.value)});
		else
			result.emplace_back(std::move(tempDecl));
		std::reverse(memoryAssignments.begin(), memoryAssignments.end());
		result += std::move(memoryAssignments);
		std::reverse(variableAssignments.begin(), variableAssignments.end());
		result += std::move(variableAssignments);
		return OptionalStatements{move(result)};
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
		if (optional<YulString> offset = m_memoryOffsetTracker(identifier->name))
			_expression = generateMemoryLoad(m_context.dialect, identifier->location, *offset);
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
