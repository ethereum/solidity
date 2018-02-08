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
 * Optimiser component that performs function inlining for arbitrary functions.
 */

#include <libjulia/optimiser/FullInliner.h>

#include <libjulia/optimiser/ASTCopier.h>
#include <libjulia/optimiser/ASTWalker.h>
#include <libjulia/optimiser/NameCollector.h>
#include <libjulia/optimiser/Semantics.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libsolidity/interface/Exceptions.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;



FullInliner::FullInliner(Block& _ast):
	m_ast(_ast)
{
	solAssert(m_ast.statements.size() >= 1, "");
	solAssert(m_ast.statements.front().type() == typeid(Block), "");
	m_nameDispenser.m_usedNames = NameCollector(m_ast).names();

	for (size_t i = 1; i < m_ast.statements.size(); ++i)
	{
		solAssert(m_ast.statements.at(i).type() == typeid(FunctionDefinition), "");
		FunctionDefinition& fun = boost::get<FunctionDefinition>(m_ast.statements.at(i));
		m_functions[fun.name] = &fun;
		m_functionsToVisit.insert(&fun);
	}
}

void FullInliner::run()
{
	solAssert(m_ast.statements[0].type() == typeid(Block), "");
	InlineModifier(*this, m_nameDispenser, "").visit(m_ast.statements[0]);
	while (!m_functionsToVisit.empty())
		handleFunction(**m_functionsToVisit.begin());
}

void FullInliner::handleFunction(FunctionDefinition& _fun)
{
	if (!m_functionsToVisit.count(&_fun))
		return;
	m_functionsToVisit.erase(&_fun);
	(InlineModifier(*this, m_nameDispenser, _fun.name))(_fun.body);
}

void InlineModifier::operator()(FunctionalInstruction& _instruction)
{
	visitArguments(_instruction.arguments);
}

void InlineModifier::operator()(FunctionCall&)
{
	solAssert(false, "Should be handled in visit() instead.");
}

void InlineModifier::operator()(ForLoop& _loop)
{
	(*this)(_loop.pre);
	// Do not visit the condition because we cannot inline there.
	(*this)(_loop.post);
	(*this)(_loop.body);
}

void InlineModifier::operator()(Block& _block)
{
	// TODO: optimize the number of moves here.
	for (size_t i = 0; i < _block.statements.size(); ++i)
	{
		visit(_block.statements.at(i));
		if (size_t length = m_statementsToPrefix.size())
		{
			_block.statements.insert(
				_block.statements.begin() + i,
				std::make_move_iterator(m_statementsToPrefix.begin()),
				std::make_move_iterator(m_statementsToPrefix.end())
			);
			i += length;
			m_statementsToPrefix.clear();
		}
	}
}

void InlineModifier::visit(Expression& _expression)
{
	if (_expression.type() != typeid(FunctionCall))
		return ASTModifier::visit(_expression);

	FunctionCall& funCall = boost::get<FunctionCall>(_expression);
	FunctionDefinition& fun = m_driver.function(funCall.functionName.name);

	m_driver.handleFunction(fun);

	// TODO: Insert good heuristic here. Perhaps implement that inside the driver.
	bool doInline = funCall.functionName.name != m_currentFunction;

	if (fun.returnVariables.size() > 1)
		doInline = false;

	{
		vector<string> argNames;
		vector<string> argTypes;
		for (auto const& arg: fun.parameters)
		{
			argNames.push_back(fun.name + "_" + arg.name);
			argTypes.push_back(arg.type);
		}
		visitArguments(funCall.arguments, argNames, argTypes, doInline);
	}

	if (!doInline)
		return;

	map<string, string> variableReplacements;
	for (size_t i = 0; i < funCall.arguments.size(); ++i)
		variableReplacements[fun.parameters[i].name] = boost::get<Identifier>(funCall.arguments[i]).name;
	if (fun.returnVariables.empty())
		_expression = noop(funCall.location);
	else
	{
		string returnVariable = fun.returnVariables[0].name;
		variableReplacements[returnVariable] = newName(fun.name + "_" + returnVariable);

		m_statementsToPrefix.emplace_back(VariableDeclaration{
			funCall.location,
			{{funCall.location, variableReplacements[returnVariable], fun.returnVariables[0].type}},
			{}
		});
		_expression = Identifier{funCall.location, variableReplacements[returnVariable]};
	}
	m_statementsToPrefix.emplace_back(BodyCopier(m_nameDispenser, fun.name + "_", variableReplacements)(fun.body));
}

void InlineModifier::visit(Statement& _statement)
{
	ASTModifier::visit(_statement);
	// Replace pop(0) expression statemets (and others) by empty blocks.
	if (_statement.type() == typeid(ExpressionStatement))
	{
		ExpressionStatement& expSt = boost::get<ExpressionStatement&>(_statement);
		if (expSt.expression.type() == typeid(FunctionalInstruction))
		{
			FunctionalInstruction& funInstr = boost::get<FunctionalInstruction&>(expSt.expression);
			if (funInstr.instruction == solidity::Instruction::POP)
				if (MovableChecker(funInstr.arguments.at(0)).movable())
					_statement = Block{expSt.location, {}};
		}
	}
}

void InlineModifier::visitArguments(
	vector<Expression>& _arguments,
	vector<string> const& _nameHints,
	vector<string> const& _types,
	bool _moveToFront
)
{
	// If one of the elements moves parts to the front, all other elements right of it
	// also have to be moved to the front to keep the order of evaluation.
	vector<Statement> prefix;
	for (size_t i = 0; i < _arguments.size(); ++i)
	{
		auto& arg = _arguments[i];
		// TODO optimize vector operations, check that it actually moves
		auto internalPrefix = visitRecursively(arg);
		if (!internalPrefix.empty())
		{
			_moveToFront = true;
			// We go through the arguments left to right, so we have to invert
			// the prefixes.
			prefix = std::move(internalPrefix) + std::move(prefix);
		}
		else if (_moveToFront)
		{
			auto location = locationOf(arg);
			string var = newName(i < _nameHints.size() ? _nameHints[i] : "");
			prefix.emplace(prefix.begin(), VariableDeclaration{
				location,
				{{TypedName{location, var, i < _types.size() ? _types[i] : ""}}},
				make_shared<Expression>(std::move(arg))
			});
			arg = Identifier{location, var};
		}
	}
	m_statementsToPrefix += std::move(prefix);
}

vector<Statement> InlineModifier::visitRecursively(Expression& _expression)
{
	vector<Statement> saved;
	saved.swap(m_statementsToPrefix);
	visit(_expression);
	saved.swap(m_statementsToPrefix);
	return saved;
}

string InlineModifier::newName(string const& _prefix)
{
	return m_nameDispenser.newName(_prefix);
}

Expression InlineModifier::noop(SourceLocation const& _location)
{
	return FunctionalInstruction{_location, solidity::Instruction::POP, {
		Literal{_location, assembly::LiteralKind::Number, "0", ""}
	}};
}

Statement BodyCopier::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_variableReplacements[var.name] = m_nameDispenser.newName(m_varNamePrefix + var.name);
	return ASTCopier::operator()(_varDecl);
}

Statement BodyCopier::operator()(FunctionDefinition const& _funDef)
{
	solAssert(false, "Function hoisting has to be done before function inlining.");
	return _funDef;
}

string BodyCopier::translateIdentifier(string const& _name)
{
	if (m_variableReplacements.count(_name))
		return m_variableReplacements.at(_name);
	else
		return _name;
}
