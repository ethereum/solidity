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
	map<YulString, map<YulString, uint64_t>> const& _memorySlots,
	uint64_t _numRequiredSlots,
	Block& _block
)
{
	StackToMemoryMover stackToMemoryMover(_context, _reservedMemory, _memorySlots, _numRequiredSlots);
	stackToMemoryMover(_block);
}

StackToMemoryMover::StackToMemoryMover(
	OptimiserStepContext& _context,
	u256 _reservedMemory,
	map<YulString, map<YulString, uint64_t>> const& _memorySlots,
	uint64_t _numRequiredSlots
):
m_context(_context),
m_reservedMemory(std::move(_reservedMemory)),
m_memorySlots(_memorySlots),
m_numRequiredSlots(_numRequiredSlots),
m_nameDispenser(_context.dispenser)
{
	auto const* evmDialect = dynamic_cast<EVMDialect const*>(&_context.dialect);
	yulAssert(
		evmDialect && evmDialect->providesObjectAccess(),
		"StackToMemoryMover can only be run on objects using the EVMDialect with object access."
	);

	if (m_memorySlots.count(YulString{}))
		// If the global scope contains variables to be moved, start with those as if it were a function.
		m_currentFunctionMemorySlots = &m_memorySlots.at(YulString{});
}

void StackToMemoryMover::operator()(FunctionDefinition& _functionDefinition)
{
	if (m_memorySlots.count(_functionDefinition.name))
	{
		map<YulString, uint64_t> const* saved = m_currentFunctionMemorySlots;
		m_currentFunctionMemorySlots = &m_memorySlots.at(_functionDefinition.name);
		for (TypedName const& param: _functionDefinition.parameters + _functionDefinition.returnVariables)
			if (m_currentFunctionMemorySlots->count(param.name))
			{
				// TODO: we cannot handle function parameters yet.
				m_currentFunctionMemorySlots = saved;
				return;
			}
		ASTModifier::operator()(_functionDefinition);
		m_currentFunctionMemorySlots = saved;
	}
}

void StackToMemoryMover::operator()(Block& _block)
{
	using OptionalStatements = std::optional<vector<Statement>>;
	if (!m_currentFunctionMemorySlots)
	{
		ASTModifier::operator()(_block);
		return;
	}
	auto containsVariableNeedingEscalation = [&](auto const& _variables) {
		return util::contains_if(_variables, [&](auto const& var) {
			return m_currentFunctionMemorySlots->count(var.name);
		});
	};
	auto rewriteAssignmentOrVariableDeclaration = [&](
		langutil::SourceLocation const& _loc,
		auto const& _variables,
		std::unique_ptr<Expression> _value
	) -> std::vector<Statement> {
		if (_variables.size() == 1)
			return generateMemoryStore(
				m_context.dialect,
				_loc,
				memoryOffset(_variables.front().name),
				_value ? *std::move(_value) : Literal{_loc, LiteralKind::Number, "0"_yulstring, {}}
			);

		VariableDeclaration tempDecl{_loc, {}, std::move(_value)};
		vector<Statement> memoryAssignments;
		vector<Statement> variableAssignments;
		for (auto& var: _variables)
		{
			YulString tempVarName = m_nameDispenser.newName(var.name);
			tempDecl.variables.emplace_back(TypedName{var.location, tempVarName, {}});

			if (m_currentFunctionMemorySlots->count(var.name))
				memoryAssignments += generateMemoryStore(
					m_context.dialect,
					_loc,
					memoryOffset(var.name),
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

void StackToMemoryMover::visit(Expression& _expression)
{
	if (
		Identifier* identifier = std::get_if<Identifier>(&_expression);
		identifier && m_currentFunctionMemorySlots && m_currentFunctionMemorySlots->count(identifier->name)
	)
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
					memoryOffset(identifier->name),
					{}
				}
			}
		};
	}
	else
		ASTModifier::visit(_expression);
}

YulString StackToMemoryMover::memoryOffset(YulString _variable)
{
	yulAssert(m_currentFunctionMemorySlots, "");
	uint64_t slot = m_currentFunctionMemorySlots->at(_variable);
	yulAssert(slot < m_numRequiredSlots, "");
	return YulString{util::toCompactHexWithPrefix(m_reservedMemory + 32 * (m_numRequiredSlots - slot - 1))};
}

