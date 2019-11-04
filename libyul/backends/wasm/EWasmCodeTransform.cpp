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

#include <libyul/backends/wasm/EWasmToText.h>
#include <libyul/optimiser/NameCollector.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

string EWasmCodeTransform::run(Dialect const& _dialect, yul::Block const& _ast)
{
	EWasmCodeTransform transform(_dialect, _ast);
	vector<wasm::FunctionDefinition> functions;

	for (auto const& statement: _ast.statements)
	{
		yulAssert(
			statement.type() == typeid(yul::FunctionDefinition),
			"Expected only function definitions at the highest level."
		);
		if (statement.type() == typeid(yul::FunctionDefinition))
			functions.emplace_back(transform.translateFunction(boost::get<yul::FunctionDefinition>(statement)));
	}

	std::vector<wasm::FunctionImport> imports;
	for (auto& imp: transform.m_functionsToImport)
		imports.emplace_back(std::move(imp.second));
	return EWasmToText().run(
		transform.m_globalVariables,
		imports,
		functions
	);
}

wasm::Expression EWasmCodeTransform::generateMultiAssignment(
	vector<string> _variableNames,
	unique_ptr<wasm::Expression> _firstValue
)
{
	yulAssert(!_variableNames.empty(), "");
	wasm::LocalAssignment assignment{move(_variableNames.front()), std::move(_firstValue)};

	if (_variableNames.size() == 1)
		return { std::move(assignment) };

	allocateGlobals(_variableNames.size() - 1);

	wasm::Block block;
	block.statements.emplace_back(move(assignment));
	for (size_t i = 1; i < _variableNames.size(); ++i)
		block.statements.emplace_back(wasm::LocalAssignment{
			move(_variableNames.at(i)),
			make_unique<wasm::Expression>(wasm::GlobalVariable{m_globalVariables.at(i - 1).variableName})
		});
	return { std::move(block) };
}

wasm::Expression EWasmCodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	vector<string> variableNames;
	for (auto const& var: _varDecl.variables)
	{
		variableNames.emplace_back(var.name.str());
		m_localVariables.emplace_back(wasm::VariableDeclaration{variableNames.back()});
	}

	if (_varDecl.value)
		return generateMultiAssignment(move(variableNames), visit(*_varDecl.value));
	else
		return wasm::BuiltinCall{"nop", {}};
}

wasm::Expression EWasmCodeTransform::operator()(Assignment const& _assignment)
{
	vector<string> variableNames;
	for (auto const& var: _assignment.variableNames)
		variableNames.emplace_back(var.name.str());
	return generateMultiAssignment(move(variableNames), visit(*_assignment.value));
}

wasm::Expression EWasmCodeTransform::operator()(ExpressionStatement const& _statement)
{
	return visitReturnByValue(_statement.expression);
}

wasm::Expression EWasmCodeTransform::operator()(FunctionalInstruction const& _f)
{
	yulAssert(false, "EVM instruction in ewasm code: " + eth::instructionInfo(_f.instruction).name);
	return {};
}

wasm::Expression EWasmCodeTransform::operator()(FunctionCall const& _call)
{
	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		if (_call.functionName.name.str().substr(0, 4) == "eth.")
		{
			yulAssert(builtin->returns.size() <= 1, "");
			// Imported function, use regular call, but mark for import.
			if (!m_functionsToImport.count(builtin->name))
			{
				wasm::FunctionImport imp{
					"ethereum",
					builtin->name.str().substr(4),
					builtin->name.str(),
					{},
					builtin->returns.empty() ? nullptr : make_unique<string>(builtin->returns.front().str())
				};
				for (auto const& param: builtin->parameters)
					imp.paramTypes.emplace_back(param.str());
				m_functionsToImport[builtin->name] = std::move(imp);
			}
		}
		else if (builtin->literalArguments)
		{
			vector<wasm::Expression> literals;
			for (auto const& arg: _call.arguments)
				literals.emplace_back(wasm::StringLiteral{boost::get<Literal>(arg).value.str()});
			return wasm::BuiltinCall{_call.functionName.name.str(), std::move(literals)};
		}
		else
			return wasm::BuiltinCall{_call.functionName.name.str(), visit(_call.arguments)};
	}

	// If this function returns multiple values, then the first one will
	// be returned in the expression itself and the others in global variables.
	// The values have to be used right away in an assignment or variable declaration,
	// so it is handled there.
	return wasm::FunctionCall{_call.functionName.name.str(), visit(_call.arguments)};
}

wasm::Expression EWasmCodeTransform::operator()(Identifier const& _identifier)
{
	return wasm::LocalVariable{_identifier.name.str()};
}

wasm::Expression EWasmCodeTransform::operator()(Literal const& _literal)
{
	u256 value = valueOfLiteral(_literal);
	yulAssert(value <= numeric_limits<uint64_t>::max(), "Literal too large: " + value.str());
	return wasm::Literal{uint64_t(value)};
}

wasm::Expression EWasmCodeTransform::operator()(If const& _if)
{
	return wasm::If{visit(*_if.condition), visit(_if.body.statements), {}};
}

wasm::Expression EWasmCodeTransform::operator()(Switch const& _switch)
{
	wasm::Block block;
	string condition = m_nameDispenser.newName("condition"_yulstring).str();
	m_localVariables.emplace_back(wasm::VariableDeclaration{condition});
	block.statements.emplace_back(wasm::LocalAssignment{condition, visit(*_switch.expression)});

	vector<wasm::Expression>* currentBlock = &block.statements;
	for (size_t i = 0; i < _switch.cases.size(); ++i)
	{
		Case const& c = _switch.cases.at(i);
		if (c.value)
		{
			wasm::BuiltinCall comparison{"i64.eq", make_vector<wasm::Expression>(
				wasm::LocalVariable{condition},
				visitReturnByValue(*c.value)
			)};
			wasm::If ifStmnt{
				make_unique<wasm::Expression>(move(comparison)),
				visit(c.body.statements),
				{}
			};
			vector<wasm::Expression>* nextBlock = nullptr;
			if (i != _switch.cases.size() - 1)
			{
				ifStmnt.elseStatements = make_unique<vector<wasm::Expression>>();
				nextBlock = ifStmnt.elseStatements.get();
			}
			currentBlock->emplace_back(move(ifStmnt));
			currentBlock = nextBlock;
		}
		else
		{
			yulAssert(i == _switch.cases.size() - 1, "Default case must be last.");
			*currentBlock += visit(c.body.statements);
		}
	}
	return { std::move(block) };
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

	wasm::Loop loop;
	loop.statements = visit(_for.pre.statements);
	loop.statements.emplace_back(wasm::BreakIf{wasm::Label{breakLabel}, make_unique<wasm::Expression>(
		wasm::BuiltinCall{"i64.eqz", make_vector<wasm::Expression>(
			visitReturnByValue(*_for.condition)
		)}
	)});
	loop.statements.emplace_back(wasm::Block{continueLabel, visit(_for.body.statements)});
	loop.statements += visit(_for.post.statements);

	return { wasm::Block{breakLabel, make_vector<wasm::Expression>(move(loop))} };
}

wasm::Expression EWasmCodeTransform::operator()(Break const&)
{
	return wasm::Break{wasm::Label{m_breakContinueLabelNames.top().first}};
}

wasm::Expression EWasmCodeTransform::operator()(Continue const&)
{
	return wasm::Break{wasm::Label{m_breakContinueLabelNames.top().second}};
}

wasm::Expression EWasmCodeTransform::operator()(Leave const&)
{
	return wasm::Return{};
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

	if (!_fun.returnVariables.empty())
	{
		// First return variable is returned directly, the others are stored
		// in globals.
		allocateGlobals(_fun.returnVariables.size() - 1);
		for (size_t i = 1; i < _fun.returnVariables.size(); ++i)
			fun.body.emplace_back(wasm::GlobalAssignment{
				m_globalVariables.at(i - 1).variableName,
				make_unique<wasm::Expression>(wasm::LocalVariable{_fun.returnVariables.at(i).name.str()})
			});
		fun.body.emplace_back(wasm::LocalVariable{_fun.returnVariables.front().name.str()});
	}
	return fun;
}

string EWasmCodeTransform::newLabel()
{
	return m_nameDispenser.newName("label_"_yulstring).str();
}

void EWasmCodeTransform::allocateGlobals(size_t _amount)
{
	while (m_globalVariables.size() < _amount)
		m_globalVariables.emplace_back(wasm::GlobalVariableDeclaration{
			m_nameDispenser.newName("global_"_yulstring).str()
		});
}
