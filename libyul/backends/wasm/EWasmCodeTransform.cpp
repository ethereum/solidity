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
/**
* Common code generator for translating Yul / inline assembly to EWasm.
*/

#include <libyul/backends/wasm/EWasmCodeTransform.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

string EWasmCodeTransform::run(yul::Block const& _ast)
{
	vector<wasm::FunctionDefinition> functions;

	for (auto const& statement: _ast.statements)
	{
		yulAssert(statement.type() == typeid(yul::FunctionDefinition), "");
		functions.emplace_back(translateFunction(boost::get<yul::FunctionDefinition>(statement)));
	}

	// TODO translate to text representation

	return {};
}

wasm::Expression EWasmCodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_localVariables.emplace_back(wasm::VariableDeclaration{var.name.str()});

	if (_varDecl.value)
	{
		// TODO otherwise, we have to work with globals.
		solUnimplementedAssert(_varDecl.variables.size() == 1, "Only single-variable assignments supported.");
		return wasm::LocalAssignment{
			_varDecl.variables.front().name.str(),
			visit(*_varDecl.value)
		};
	}
	else
		// TODO this could be handled better.
		return wasm::BuiltinCall{"nop", {}};
}

wasm::Expression EWasmCodeTransform::operator()(Assignment const& _assignment)
{
	solUnimplementedAssert(_assignment.variableNames.size() == 1, "Only single-variable assignments supported.");
	return wasm::LocalAssignment{
		_assignment.variableNames.front().name.str(),
		visit(*_assignment.value)
	};
}

wasm::Expression EWasmCodeTransform::operator()(StackAssignment const&)
{
	yulAssert(false, "");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(ExpressionStatement const& _statement)
{
	return visitReturnByValue(_statement.expression);
}

wasm::Expression EWasmCodeTransform::operator()(Label const&)
{
	yulAssert(false, "");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(FunctionalInstruction const&)
{
	yulAssert(false, "");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(FunctionCall const& _call)
{
	if (m_dialect.builtin(_call.functionName.name))
		return wasm::BuiltinCall{_call.functionName.name.str(), visit(_call.arguments)};
	else
		return wasm::FunctionCall{_call.functionName.name.str(), visit(_call.arguments)};
}

wasm::Expression EWasmCodeTransform::operator()(Identifier const& _identifier)
{
	return wasm::Identifier{_identifier.name.str()};
}

wasm::Expression EWasmCodeTransform::operator()(Literal const& _literal)
{
	u256 value = valueOfLiteral(_literal);
	yulAssert(value <= numeric_limits<uint64_t>::max(), "");
	return wasm::Literal{uint64_t(value)};
}

wasm::Expression EWasmCodeTransform::operator()(yul::Instruction const&)
{
	yulAssert(false, "");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(If const& _if)
{
	return wasm::If{visit(*_if.condition), visit(_if.body.statements)};
}

wasm::Expression EWasmCodeTransform::operator()(Switch const&)
{
	solUnimplementedAssert(false, "");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(FunctionDefinition const&)
{
	yulAssert(false, "Should not have visited here.");
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(ForLoop const& _for)
{
	string breakLabel = newLabel();
	string continueLabel = newLabel();
	m_breakContinueLabelNames.push({breakLabel, continueLabel});

	// The AST is constructed in this weird way because of some strange
	// problem with move semantics.
	wasm::BuiltinCall loopCondition{"i64.eqz", {}};
	loopCondition.arguments.emplace_back(visitReturnByValue(*_for.condition));

	wasm::BuiltinCall conditionCheck{"br_if", {}};
	conditionCheck.arguments.emplace_back(wasm::Label{breakLabel});
	conditionCheck.arguments.emplace_back(move(loopCondition));

	wasm::Loop loop;
	loop.statements = visit(_for.pre.statements);
	loop.statements.emplace_back(move(conditionCheck));
	loop.statements.emplace_back(wasm::Block{continueLabel, visit(_for.body.statements)});
	loop.statements += visit(_for.post.statements);

	wasm::Block breakBlock{breakLabel, {}};
	breakBlock.statements.emplace_back(move(loop));
	return move(breakBlock);
}

wasm::Expression EWasmCodeTransform::operator()(Break const&)
{
	return wasm::Break{wasm::Label{m_breakContinueLabelNames.top().first}};
}

wasm::Expression EWasmCodeTransform::operator()(Continue const&)
{
	return wasm::Continue{wasm::Label{m_breakContinueLabelNames.top().second}};
}

wasm::Expression EWasmCodeTransform::operator()(Block const& _block)
{
	return wasm::Block{{}, visit(_block.statements)};
}

unique_ptr<wasm::Expression> EWasmCodeTransform::visit(yul::Expression const& _expression)
{
	return make_unique<wasm::Expression>(boost::apply_visitor(*this, _expression));
}

wasm::Expression EWasmCodeTransform::visitReturnByValue(yul::Expression const& _expression)
{
	return boost::apply_visitor(*this, _expression);
}

vector<wasm::Expression> EWasmCodeTransform::visit(vector<yul::Expression> const& _expressions)
{
	vector<wasm::Expression> ret;
	for (auto const& e: _expressions)
		ret.emplace_back(visitReturnByValue(e));
	return ret;
}

wasm::Expression EWasmCodeTransform::visit(yul::Statement const& _statement)
{
	return boost::apply_visitor(*this, _statement);
}

vector<wasm::Expression> EWasmCodeTransform::visit(vector<yul::Statement> const& _statements)
{
	vector<wasm::Expression> ret;
	for (auto const& s: _statements)
		ret.emplace_back(visit(s));
	return ret;
}

wasm::FunctionDefinition EWasmCodeTransform::translateFunction(yul::FunctionDefinition const& _fun)
{
	wasm::FunctionDefinition fun;
	fun.name = _fun.name.str();
	for (auto const& param: _fun.parameters)
		fun.parameterNames.emplace_back(param.name.str());
	for (auto const& retParam: _fun.returnVariables)
		fun.locals.emplace_back(wasm::VariableDeclaration{retParam.name.str()});
	fun.returns = !_fun.returnVariables.empty();

	yulAssert(m_localVariables.empty(), "");
	fun.body = visit(_fun.body.statements);
	fun.locals += m_localVariables;

	m_localVariables.clear();
	yulAssert(_fun.returnVariables.size() <= 1, "");
	if (_fun.returnVariables.size() == 1)
		fun.body.emplace_back(wasm::Identifier{_fun.returnVariables.front().name.str()});
	return fun;
}

string EWasmCodeTransform::newLabel()
{
	// TODO this should not clash with other identifiers!
	return "label_" + to_string(++m_labelCounter);
}
