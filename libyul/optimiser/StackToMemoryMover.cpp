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
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/AST.h>

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
	VariableMemoryOffsetTracker memoryOffsetTracker(_reservedMemory, _memorySlots, _numRequiredSlots);
	StackToMemoryMover stackToMemoryMover(_context, memoryOffsetTracker);
	stackToMemoryMover(_block);
}

StackToMemoryMover::StackToMemoryMover(
	OptimiserStepContext& _context,
	VariableMemoryOffsetTracker const& _memoryOffsetTracker
):
m_context(_context),
m_memoryOffsetTracker(_memoryOffsetTracker),
m_nameDispenser(_context.dispenser)
{
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackToMemoryMover can only be run on objects using the EVMDialect with object access."
	);
}

void StackToMemoryMover::operator()(FunctionDefinition& _functionDefinition)
{
	for (TypedName const& param: _functionDefinition.parameters + _functionDefinition.returnVariables)
		if (m_memoryOffsetTracker(param.name))
		{
			// TODO: we cannot handle function parameters yet.
			return;
		}
	ASTModifier::operator()(_functionDefinition);
}

void StackToMemoryMover::operator()(Block& _block)
{
	using OptionalStatements = std::optional<vector<Statement>>;
	auto rewriteAssignmentOrVariableDeclaration = [&](
		auto& _stmt,
		auto const& _variables
	) -> OptionalStatements {
		using StatementType = decay_t<decltype(_stmt)>;
		if (_stmt.value)
			visit(*_stmt.value);
		bool leftHandSideNeedsMoving = util::contains_if(_variables, [&](auto const& var) {
			return m_memoryOffsetTracker(var.name);
		});
		if (!leftHandSideNeedsMoving)
			return {};

		langutil::SourceLocation loc = _stmt.location;

		if (_variables.size() == 1)
		{
			optional<YulString> offset = m_memoryOffsetTracker(_variables.front().name);
			yulAssert(offset, "");
			return generateMemoryStore(
				m_context.dialect,
				loc,
				*offset,
				_stmt.value ? *std::move(_stmt.value) : Literal{loc, LiteralKind::Number, "0"_yulstring, {}}
			);
		}

		VariableDeclaration tempDecl{loc, {}, std::move(_stmt.value)};
		vector<Statement> memoryAssignments;
		vector<Statement> variableAssignments;
		for (auto& var: _variables)
		{
			YulString tempVarName = m_nameDispenser.newName(var.name);
			tempDecl.variables.emplace_back(TypedName{var.location, tempVarName, {}});

			if (optional<YulString> offset = m_memoryOffsetTracker(var.name))
				memoryAssignments += generateMemoryStore(
					m_context.dialect,
					loc,
					*offset,
					Identifier{loc, tempVarName}
				);
			else
				variableAssignments.emplace_back(StatementType{
					loc, {move(var)},
					make_unique<Expression>(Identifier{loc, tempVarName})
				});
		}
		std::vector<Statement> result;
		result.emplace_back(std::move(tempDecl));
		std::reverse(memoryAssignments.begin(), memoryAssignments.end());
		result += std::move(memoryAssignments);
		std::reverse(variableAssignments.begin(), variableAssignments.end());
		result += std::move(variableAssignments);
		return OptionalStatements{move(result)};
	};

	util::iterateReplacing(
		_block.statements,
		[&](Statement& _statement)
		{
			return std::visit(util::GenericVisitor{
				[&](Assignment& _assignment) -> OptionalStatements
				{
					return rewriteAssignmentOrVariableDeclaration(_assignment, _assignment.variableNames);
				},
				[&](VariableDeclaration& _varDecl) -> OptionalStatements
				{
					return rewriteAssignmentOrVariableDeclaration(_varDecl, _varDecl.variables);
				},
				[&](auto& _stmt) -> OptionalStatements { (*this)(_stmt); return {}; }
			}, _statement);
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
