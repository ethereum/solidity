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
/**
 * Stack layout generator for Yul to EVM code generation.
 */

#include <libyul/backends/evm/DirectStackLayoutGenerator.h>

#include <libyul/AST.h>
#include <libyul/Utilities.h>

#include <libsolutil/cxx20.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/drop_exactly.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

DirectStackLayoutGenerator::Context DirectStackLayoutGenerator::run(AsmAnalysisInfo const& _analysisInfo, Dialect const& _dialect, Block const& _block)
{
	/*
	yulAssert(_block.statements.size() > 0, "");
	yulAssert(holds_alternative<Block>(_block.statements.front()), "");

	{
		DirectStackLayoutGenerator generator{_analysisInfo, _dialect};
		generator(get<Block>(_block.statements.front()));
	}
	for (Statement const& statement: _block.statements | ranges::views::drop_exactly(1))
	{
		FunctionDefinition const* functionDefinition = get_if<FunctionDefinition>(&statement);
		yulAssert(functionDefinition, "");
		DirectStackLayoutGenerator generator{_analysisInfo, _dialect};
		generator(*functionDefinition);
	}*/
	Context context;
	DirectStackLayoutGenerator generator{context, _analysisInfo, _dialect};
	generator(_block);
	return context;
}

void DirectStackLayoutGenerator::operator()(Block const& _block)
{
	ScopedSaveAndRestore saveScope(m_scope, m_info.scopes.at(&_block).get());
	for (auto const& statement: _block.statements)
		if (auto const* function = get_if<FunctionDefinition>(&statement))
			registerFunction(*function);
	for (Statement const& statement: _block.statements | ranges::views::reverse)
		visit(statement);

	m_context.layout.blockInfos[&_block].entry = m_stack;
}

void DirectStackLayoutGenerator::operator()(VariableDeclaration const& _variableDeclaration)
{
	auto declaredVariables = _variableDeclaration.variables | ranges::views::transform([&](TypedName const& _var) {
		return VariableSlot{lookupVariable(_var.name), _var.debugData};
	}) | ranges::to<vector<VariableSlot>>;

	visitAssignmentOrDeclaration(
		debugDataOf(_variableDeclaration),
		declaredVariables,
		_variableDeclaration.value ? _variableDeclaration.value.get() : nullptr
	);
}

void DirectStackLayoutGenerator::operator()(Assignment const& _assignment)
{
	auto assignedVariables = _assignment.variableNames | ranges::views::transform([&](Identifier const& _var) {
		return VariableSlot{lookupVariable(_var.name), _var.debugData};
	}) | ranges::to<vector<VariableSlot>>;
	visitAssignmentOrDeclaration(
		debugDataOf(_assignment),
		assignedVariables,
		_assignment.value.get()
	);
}

void DirectStackLayoutGenerator::operator()(Literal const& _literal)
{
	m_stack.emplace_back(LiteralSlot{valueOfLiteral(_literal), debugDataOf(_literal)});
}

void DirectStackLayoutGenerator::operator()(Identifier const& _identifier)
{
	m_stack.emplace_back(VariableSlot{lookupVariable(_identifier.name), debugDataOf(_identifier)});
}

void DirectStackLayoutGenerator::operator()(FunctionCall const& _funCall)
{
	yulAssert(visitFunctionCall(_funCall) == 1, "");
}

void DirectStackLayoutGenerator::operator()(ExpressionStatement const& _expressionStatement)
{
	std::visit(util::GenericVisitor{
		[&](FunctionCall const& _call) {
			yulAssert(visitFunctionCall(_call) == 0, "");
		},
		[&](auto const&) { yulAssert(false, ""); }
	}, _expressionStatement.expression);
}

void DirectStackLayoutGenerator::operator()(If const& _if)
{
	DirectStackLayoutGenerator bodyGenerator{m_context, m_info, m_dialect, m_stack};
	bodyGenerator(_if.body);
	m_stack += move(bodyGenerator.m_stack);
	visit(*_if.condition);
}
void DirectStackLayoutGenerator::operator()(Switch const& _switch)
{
	Stack combinedStack;
	for (auto const& switchCase: _switch.cases)
	{
		DirectStackLayoutGenerator bodyGenerator{m_context, m_info, m_dialect, m_stack};
		bodyGenerator(switchCase.body);
		for (auto const& slot: bodyGenerator.m_stack)
			if (!util::contains(combinedStack, slot))
				combinedStack.emplace_back(slot);
	}
	m_stack += move(combinedStack);
	visit(*_switch.expression);
}
void DirectStackLayoutGenerator::operator()(ForLoop const& _forLoop)
{
	yulAssert(_forLoop.pre.statements.empty(), "");
	DirectStackLayoutGenerator bodyGenerator{m_context, m_info, m_dialect, m_stack};
	bodyGenerator(_forLoop.post);
	bodyGenerator(_forLoop.body);
	m_stack += move(bodyGenerator.m_stack);
	visit(*_forLoop.condition);
}

void DirectStackLayoutGenerator::operator()(FunctionDefinition const& _function)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));

	auto const& functionInfo = m_context.functionInfo.at(&function);

	yulAssert(m_info.scopes.at(&_function.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	yulAssert(virtualFunctionScope, "");

	DirectStackLayoutGenerator bodyGenerator{m_context, m_info, m_dialect, m_stack};
	bodyGenerator.m_stack = functionInfo.returnVariables | ranges::views::transform([](auto const& _varSlot){
		return StackSlot{_varSlot};
	}) | ranges::to<Stack>;
	bodyGenerator.m_stack.emplace_back(FunctionReturnLabelSlot{function});
	bodyGenerator(_function.body);
}

void DirectStackLayoutGenerator::visit(Statement const& _statement)
{
	ASTWalker::visit(_statement);
	m_context.layout.statementInfos[&_statement] = m_stack;
}

size_t DirectStackLayoutGenerator::visitFunctionCall(FunctionCall const& _call)
{
	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		for (auto&& [idx, arg]: _call.arguments | ranges::views::enumerate | ranges::views::reverse)
			if (!builtin->literalArgument(idx).has_value())
				std::visit(*this, arg);
		return builtin->returns.size();
	}
	else
	{
		m_stack.emplace_back(FunctionCallReturnLabelSlot{_call});
		for (auto const& arg: _call.arguments | ranges::views::reverse)
			std::visit(*this, arg);
		Scope::Function const& function = lookupFunction(_call.functionName.name);
		return function.returns.size();
	}
}

void DirectStackLayoutGenerator::visitAssignmentOrDeclaration(
	std::shared_ptr<DebugData const> _debugData,
	vector<VariableSlot> const& _variables,
	Expression const* _expression
)
{
	// TODO: reproduce proper createIdealLayout here
	cxx20::erase_if(m_stack, [&](StackSlot const& slot) {
		if (auto const* varSlot = get_if<VariableSlot>(&slot))
			if (util::contains(_variables, *varSlot))
				return true;
		return false;
	});
	if (!_expression)
	{
		m_stack += Stack(_variables.size(), LiteralSlot{0, _debugData});
		return;
	}

	if (_variables.size() == 1 && !holds_alternative<FunctionCall>(*_expression))
	{
		visit(*_expression);
		return;
	}

	auto const* call = get_if<FunctionCall>(_expression);
	yulAssert(call, "");
	yulAssert(visitFunctionCall(*call) == _variables.size(), "");
}

Scope::Function const& DirectStackLayoutGenerator::lookupFunction(YulString _name) const
{
	Scope::Function const* function = nullptr;
	yulAssert(m_scope->lookup(_name, util::GenericVisitor{
		[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
		[&](Scope::Function& _function) { function = &_function; }
	}), "Function name not found.");
	yulAssert(function, "");
	return *function;
}

Scope::Variable const& DirectStackLayoutGenerator::lookupVariable(YulString _name) const
{
	yulAssert(m_scope, "");
	Scope::Variable const* var = nullptr;
	if (m_scope->lookup(_name, util::GenericVisitor{
		[&](Scope::Variable& _var) { var = &_var; },
		[](Scope::Function&)
		{
			yulAssert(false, "Function not removed during desugaring.");
		}
	}))
	{
		yulAssert(var, "");
		return *var;
	};
	yulAssert(false, "External identifier access unimplemented.");
}

void DirectStackLayoutGenerator::registerFunction(FunctionDefinition const& _function)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));
	m_context.functionList.emplace_back(&function);

	yulAssert(m_info.scopes.at(&_function.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	yulAssert(virtualFunctionScope, "");

	bool inserted = m_context.functionInfo.emplace(std::make_pair(&function, FunctionInfo{
		_function.debugData,
		function,
		_function.parameters | ranges::views::transform([&](auto const& _param) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_param.name)),
				_param.debugData
			};
		}) | ranges::to<vector>,
		_function.returnVariables | ranges::views::transform([&](auto const& _retVar) {
			return VariableSlot{
				std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(_retVar.name)),
				_retVar.debugData
			};
		}) | ranges::to<vector>
	})).second;
	yulAssert(inserted);
}
