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
* Common code generator for translating Yul / inline assembly to Wasm.
*/

#include <libyul/backends/wasm/WasmCodeTransform.h>

#include <libyul/backends/wasm/WasmDialect.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <liblangutil/Exceptions.h>

#include <optional>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

wasm::Module WasmCodeTransform::run(Dialect const& _dialect, yul::Block const& _ast)
{
	wasm::Module module;

	TypeInfo typeInfo(_dialect, _ast);
	WasmCodeTransform transform(_dialect, _ast, typeInfo);

	for (auto const& statement: _ast.statements)
	{
		yulAssert(
			holds_alternative<yul::FunctionDefinition>(statement),
			"Expected only function definitions at the highest level."
		);
		if (holds_alternative<yul::FunctionDefinition>(statement))
			module.functions.emplace_back(transform.translateFunction(std::get<yul::FunctionDefinition>(statement)));
	}

	for (auto& imp: transform.m_functionsToImport)
		module.imports.emplace_back(std::move(imp.second));
	module.globals = transform.m_globalVariables;

	return module;
}

wasm::Expression WasmCodeTransform::generateMultiAssignment(
	vector<string> _variableNames,
	unique_ptr<wasm::Expression> _firstValue
)
{
	yulAssert(!_variableNames.empty(), "");
	wasm::LocalAssignment assignment{move(_variableNames.front()), std::move(_firstValue)};

	if (_variableNames.size() == 1)
		return { std::move(assignment) };

	vector<wasm::Type> typesForGlobals;
	for (size_t i = 1; i < _variableNames.size(); ++i)
		typesForGlobals.push_back(translatedType(m_typeInfo.typeOfVariable(YulString(_variableNames[i]))));
	vector<size_t> allocatedIndices = allocateGlobals(typesForGlobals);
	yulAssert(allocatedIndices.size() == _variableNames.size() - 1, "");

	wasm::Block block;
	block.statements.emplace_back(move(assignment));
	for (size_t i = 1; i < _variableNames.size(); ++i)
		block.statements.emplace_back(wasm::LocalAssignment{
			move(_variableNames.at(i)),
			make_unique<wasm::Expression>(wasm::GlobalVariable{m_globalVariables.at(allocatedIndices[i - 1]).variableName})
		});
	return { std::move(block) };
}

wasm::Expression WasmCodeTransform::operator()(yul::VariableDeclaration const& _varDecl)
{
	vector<string> variableNames;
	for (auto const& var: _varDecl.variables)
	{
		variableNames.emplace_back(var.name.str());
		m_localVariables.emplace_back(wasm::VariableDeclaration{variableNames.back(), translatedType(var.type)});
	}

	if (_varDecl.value)
		return generateMultiAssignment(move(variableNames), visit(*_varDecl.value));
	else
		return wasm::BuiltinCall{"nop", {}};
}

wasm::Expression WasmCodeTransform::operator()(yul::Assignment const& _assignment)
{
	vector<string> variableNames;
	for (auto const& var: _assignment.variableNames)
		variableNames.emplace_back(var.name.str());
	return generateMultiAssignment(move(variableNames), visit(*_assignment.value));
}

wasm::Expression WasmCodeTransform::operator()(yul::ExpressionStatement const& _statement)
{
	return visitReturnByValue(_statement.expression);
}

void WasmCodeTransform::importBuiltinFunction(BuiltinFunction const* _builtin, string const& _module, string const& _externalName, string const& _internalName)
{
	yulAssert(_builtin, "");
	yulAssert(_builtin->returns.size() <= 1, "");
	// Imported function, use regular call, but mark for import.
	YulString internalName(_internalName);
	if (!m_functionsToImport.count(internalName))
	{
		wasm::FunctionImport imp{
			_module,
			_externalName,
			_internalName,
			{},
			_builtin->returns.empty() ? nullopt : make_optional<wasm::Type>(translatedType(_builtin->returns.front()))
		};
		for (auto const& param: _builtin->parameters)
			imp.paramTypes.emplace_back(translatedType(param));
		m_functionsToImport[internalName] = move(imp);
	}
}

wasm::Expression WasmCodeTransform::operator()(yul::FunctionCall const& _call)
{
	if (BuiltinFunction const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		if (_call.functionName.name.str().substr(0, 6) == "debug.")
			importBuiltinFunction(builtin, "debug", builtin->name.str().substr(6), builtin->name.str());
		else if (_call.functionName.name.str().substr(0, 4) == "eth.")
			importBuiltinFunction(builtin, "ethereum", builtin->name.str().substr(4), builtin->name.str());
		else
		{
			vector<wasm::Expression> arguments;
			for (size_t i = 0; i < _call.arguments.size(); i++)
				if (builtin->literalArgument(i))
				{
					yulAssert(builtin->literalArgument(i) == LiteralKind::String, "");
					arguments.emplace_back(wasm::StringLiteral{std::get<Literal>(_call.arguments[i]).value.str()});
				}
				else
					arguments.emplace_back(visitReturnByValue(_call.arguments[i]));

			return wasm::BuiltinCall{_call.functionName.name.str(), std::move(arguments)};
		}
	}

	// If this function returns multiple values, then the first one will
	// be returned in the expression itself and the others in global variables.
	// The values have to be used right away in an assignment or variable declaration,
	// so it is handled there.

	return wasm::FunctionCall{_call.functionName.name.str(), visit(_call.arguments)};
}

wasm::Expression WasmCodeTransform::operator()(yul::Identifier const& _identifier)
{
	return wasm::LocalVariable{_identifier.name.str()};
}

wasm::Expression WasmCodeTransform::operator()(yul::Literal const& _literal)
{
	return makeLiteral(translatedType(_literal.type), valueOfLiteral(_literal));
}

wasm::Expression WasmCodeTransform::operator()(yul::If const& _if)
{
	yul::Type conditionType = m_typeInfo.typeOf(*_if.condition);

	wasm::Expression condition;
	if (conditionType == "i32"_yulstring)
		condition = visitReturnByValue(*_if.condition);
	else if (conditionType == "i64"_yulstring)
	{
		vector<wasm::Expression> args;
		args.emplace_back(visitReturnByValue(*_if.condition));
		args.emplace_back(makeLiteral(translatedType("i64"_yulstring), 0));

		// NOTE: `if` in wasm requires an i32 argument
		condition = wasm::BuiltinCall{"i64.ne", std::move(args)};
	}
	else
		yulAssert(false, "Invalid condition type");

	return wasm::If{make_unique<wasm::Expression>(move(condition)), visit(_if.body.statements), {}};
}

wasm::Expression WasmCodeTransform::operator()(yul::Switch const& _switch)
{
	yul::Type expressionType = m_typeInfo.typeOf(*_switch.expression);
	YulString eq_instruction = YulString(expressionType.str() + ".eq");
	yulAssert(WasmDialect::instance().builtin(eq_instruction), "");

	wasm::Block block;
	string condition = m_nameDispenser.newName("condition"_yulstring).str();
	m_localVariables.emplace_back(wasm::VariableDeclaration{condition, translatedType(expressionType)});
	block.statements.emplace_back(wasm::LocalAssignment{condition, visit(*_switch.expression)});

	vector<wasm::Expression>* currentBlock = &block.statements;
	for (size_t i = 0; i < _switch.cases.size(); ++i)
	{
		Case const& c = _switch.cases.at(i);
		if (c.value)
		{
			wasm::BuiltinCall comparison{eq_instruction.str(), make_vector<wasm::Expression>(
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

wasm::Expression WasmCodeTransform::operator()(yul::FunctionDefinition const&)
{
	yulAssert(false, "Should not have visited here.");
	return {};
}

wasm::Expression WasmCodeTransform::operator()(yul::ForLoop const& _for)
{
	string breakLabel = newLabel();
	string continueLabel = newLabel();
	m_breakContinueLabelNames.push({breakLabel, continueLabel});

	yul::Type conditionType = m_typeInfo.typeOf(*_for.condition);
	YulString eqz_instruction = YulString(conditionType.str() + ".eqz");
	yulAssert(WasmDialect::instance().builtin(eqz_instruction), "");

	std::vector<wasm::Expression> statements = visit(_for.pre.statements);

	wasm::Loop loop;
	loop.labelName = newLabel();
	loop.statements.emplace_back(wasm::BranchIf{wasm::Label{breakLabel}, make_unique<wasm::Expression>(
		wasm::BuiltinCall{eqz_instruction.str(), make_vector<wasm::Expression>(
			visitReturnByValue(*_for.condition)
		)}
	)});
	loop.statements.emplace_back(wasm::Block{continueLabel, visit(_for.body.statements)});
	loop.statements += visit(_for.post.statements);
	loop.statements.emplace_back(wasm::Branch{wasm::Label{loop.labelName}});

	statements += make_vector<wasm::Expression>(move(loop));
	return wasm::Block{breakLabel, move(statements)};
}

wasm::Expression WasmCodeTransform::operator()(yul::Break const&)
{
	yulAssert(m_breakContinueLabelNames.size() > 0, "");
	return wasm::Branch{wasm::Label{m_breakContinueLabelNames.top().first}};
}

wasm::Expression WasmCodeTransform::operator()(yul::Continue const&)
{
	yulAssert(m_breakContinueLabelNames.size() > 0, "");
	return wasm::Branch{wasm::Label{m_breakContinueLabelNames.top().second}};
}

wasm::Expression WasmCodeTransform::operator()(yul::Leave const&)
{
	yulAssert(!m_functionBodyLabel.empty(), "");
	return wasm::Branch{wasm::Label{m_functionBodyLabel}};
}

wasm::Expression WasmCodeTransform::operator()(yul::Block const& _block)
{
	return wasm::Block{{}, visit(_block.statements)};
}

unique_ptr<wasm::Expression> WasmCodeTransform::visit(yul::Expression const& _expression)
{
	return make_unique<wasm::Expression>(std::visit(*this, _expression));
}

wasm::Expression WasmCodeTransform::visitReturnByValue(yul::Expression const& _expression)
{
	return std::visit(*this, _expression);
}

vector<wasm::Expression> WasmCodeTransform::visit(vector<yul::Expression> const& _expressions)
{
	vector<wasm::Expression> ret;
	for (auto const& e: _expressions)
		ret.emplace_back(visitReturnByValue(e));
	return ret;
}

wasm::Expression WasmCodeTransform::visit(yul::Statement const& _statement)
{
	return std::visit(*this, _statement);
}

vector<wasm::Expression> WasmCodeTransform::visit(vector<yul::Statement> const& _statements)
{
	vector<wasm::Expression> ret;
	for (auto const& s: _statements)
		ret.emplace_back(visit(s));
	return ret;
}

wasm::FunctionDefinition WasmCodeTransform::translateFunction(yul::FunctionDefinition const& _fun)
{
	wasm::FunctionDefinition fun;
	fun.name = _fun.name.str();
	for (auto const& param: _fun.parameters)
		fun.parameters.push_back({param.name.str(), translatedType(param.type)});
	for (auto const& retParam: _fun.returnVariables)
		fun.locals.emplace_back(wasm::VariableDeclaration{retParam.name.str(), translatedType(retParam.type)});
	if (!_fun.returnVariables.empty())
		fun.returnType = translatedType(_fun.returnVariables[0].type);

	yulAssert(m_localVariables.empty(), "");
	yulAssert(m_functionBodyLabel.empty(), "");
	m_functionBodyLabel = newLabel();
	fun.body.emplace_back(wasm::Expression(wasm::Block{
		m_functionBodyLabel,
		visit(_fun.body.statements)
	}));
	fun.locals += m_localVariables;

	m_localVariables.clear();
	m_functionBodyLabel = {};

	if (!_fun.returnVariables.empty())
	{
		// First return variable is returned directly, the others are stored
		// in globals.
		vector<wasm::Type> typesForGlobals;
		for (size_t i = 1; i < _fun.returnVariables.size(); ++i)
			typesForGlobals.push_back(translatedType(_fun.returnVariables[i].type));
		vector<size_t> allocatedIndices = allocateGlobals(typesForGlobals);
		yulAssert(allocatedIndices.size() == _fun.returnVariables.size() - 1, "");

		for (size_t i = 1; i < _fun.returnVariables.size(); ++i)
			fun.body.emplace_back(wasm::GlobalAssignment{
				m_globalVariables.at(allocatedIndices[i - 1]).variableName,
				make_unique<wasm::Expression>(wasm::LocalVariable{_fun.returnVariables.at(i).name.str()})
			});
		fun.body.emplace_back(wasm::LocalVariable{_fun.returnVariables.front().name.str()});
	}
	return fun;
}

string WasmCodeTransform::newLabel()
{
	return m_nameDispenser.newName("label_"_yulstring).str();
}

vector<size_t> WasmCodeTransform::allocateGlobals(vector<wasm::Type> const& _typesForGlobals)
{
	map<wasm::Type, size_t> availableGlobals;
	for (wasm::GlobalVariableDeclaration const& global: m_globalVariables)
		++availableGlobals[global.type];

	map<wasm::Type, size_t> neededGlobals;
	for (wasm::Type const& type: _typesForGlobals)
		++neededGlobals[type];

	for (auto [type, neededGlobalCount]: neededGlobals)
		while (availableGlobals[type] < neededGlobalCount)
		{
			m_globalVariables.emplace_back(wasm::GlobalVariableDeclaration{
				m_nameDispenser.newName("global_"_yulstring).str(),
				type,
			});

			++availableGlobals[type];
		}

	vector<size_t> allocatedIndices;
	map<wasm::Type, size_t> nextGlobal;
	for (wasm::Type const& type: _typesForGlobals)
	{
		while (m_globalVariables[nextGlobal[type]].type != type)
			++nextGlobal[type];

		allocatedIndices.push_back(nextGlobal[type]++);
	}

	yulAssert(all_of(
		allocatedIndices.begin(),
		allocatedIndices.end(),
		[this](size_t index){ return index < m_globalVariables.size(); }
	), "");
	yulAssert(allocatedIndices.size() == set<size_t>(allocatedIndices.begin(), allocatedIndices.end()).size(), "Indices not unique");
	yulAssert(allocatedIndices.size() == _typesForGlobals.size(), "");
	return allocatedIndices;
}

wasm::Type WasmCodeTransform::translatedType(yul::Type _yulType)
{
	if (_yulType == "i32"_yulstring)
		return wasm::Type::i32;
	else if (_yulType == "i64"_yulstring)
		return wasm::Type::i64;
	else
		yulAssert(false, "This Yul type does not have a corresponding type in Wasm.");
}

wasm::Literal WasmCodeTransform::makeLiteral(wasm::Type _type, u256 _value)
{
	if (_type == wasm::Type::i32)
	{
		yulAssert(_value <= numeric_limits<uint32_t>::max(), "Literal too large: " + _value.str());
		return wasm::Literal{static_cast<uint32_t>(_value)};
	}
	else if (_type == wasm::Type::i64)
	{
		yulAssert(_value <= numeric_limits<uint64_t>::max(), "Literal too large: " + _value.str());
		return wasm::Literal{static_cast<uint64_t>(_value)};
	}
	else
		yulAssert(false, "Invalid Wasm literal type");
}
