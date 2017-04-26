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
 * @author Christian <c@ethdev.com>
 * @date 2017
 * Desugars assembly, i.e. converts the parsed ast and removes functions, switches and loops.
 */

#include <libsolidity/inlineasm/AsmDesugar.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScope.h>

#include <libsolidity/interface/Utils.h>

#include <boost/range/algorithm/transform.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;


Statement ASTNodeReplacement::asStatement()
{
	assertThrow(statements.size() == 1, DesugaringError, "Tried to convert multiple statements to single.");
	return statements.front();
}

void ASTNodeReplacement::moveAppend(std::vector<Statement>& _target)
{
	std::move(statements.begin(), statements.end(), std::back_inserter(_target));
}

bool ASTNodeReplacement::isMultipleOrBlock() const
{
	assertThrow(statements.size() > 0, DesugaringError, "Invalid empty replacement.");
	return statements.size() > 1 || statements.front().type() == typeid(assembly::Block);
}


ASTNodeReplacement::operator assembly::Statement() &&
{
	assertThrow(statements.size() == 1, DesugaringError, "Tried to convert multiple statements to single.");
	return statements.front();
}

Block AsmDesugar::run(Block const& _in)
{
	return boost::get<Block>((*this)(_in).asStatement());
}

ASTNodeReplacement AsmDesugar::operator()(assembly::Instruction const& _instruction)
{
	return {_instruction};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::Literal const& _literal)
{
	return {_literal};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::Identifier const& _identifier)
{
	return {_identifier};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionalInstruction const& _functionalInstruction)
{
	bool haveToExplode = false;
	vector<ASTNodeReplacement> arguments;
	for (auto const& arg: _functionalInstruction.arguments)
	{
		arguments.push_back(std::move(boost::apply_visitor(*this, arg)));
		if (arguments.back().isMultipleOrBlock())
			haveToExplode = true;
	}
	if (haveToExplode)
	{
		vector<assembly::Statement> statements;
		for (ASTNodeReplacement& rep: arguments | boost::adaptors::reversed)
			rep.moveAppend(statements);
		statements.push_back(_functionalInstruction.instruction);
		return {std::move(statements)};
	}
	else
	{
		FunctionalInstruction result;
		result.location = _functionalInstruction.location;
		result.instruction = _functionalInstruction.instruction;
		std::move(arguments.begin(), arguments.end(), std::back_inserter(result.arguments));
		return {std::move(result)};
	}
}

ASTNodeReplacement AsmDesugar::operator()(assembly::Label const& _label)
{
	return {_label};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::Assignment const& _assignment)
{
	return {_assignment};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionalAssignment const& _functionalAssignment)
{
	ASTNodeReplacement value = boost::apply_visitor(*this, *_functionalAssignment.value);
	if (value.isMultipleOrBlock())
	{
		vector<assembly::Statement> statements;
		value.moveAppend(statements);
		statements.push_back(Assignment{_functionalAssignment.location, _functionalAssignment.variableName});
		return {std::move(statements)};
	}
	else
	{
		FunctionalAssignment result;
		result.location = _functionalAssignment.location;
		result.variableName = _functionalAssignment.variableName;
		result.value = make_shared<Statement>(value.asStatement());
		return {std::move(result)};
	}
}

ASTNodeReplacement AsmDesugar::operator()(assembly::VariableDeclaration const& _variableDeclaration)
{
	//@TODO handle special case of `let x := f()` where return label and variable introduction label can be combined.
	ASTNodeReplacement value = boost::apply_visitor(*this, *_variableDeclaration.value);
	if (value.isMultipleOrBlock())
	{
		vector<Statement> statements;
		value.moveAppend(statements);
		statements.push_back(Label{
			_variableDeclaration.location,
			generateIdentifier("$" + _variableDeclaration.name + "_"),
			{_variableDeclaration.name}
		});
		return {std::move(statements)};
	}
	else
	{
		VariableDeclaration result;
		result.location = _variableDeclaration.location;
		result.name = _variableDeclaration.name;
		result.value = make_shared<Statement>(value.asStatement());
		return {std::move(result)};
	}
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionDefinition const& _functionDefinition)
{
	auto loc = _functionDefinition.location;

	string afterFunction = generateIdentifier("$after_" + _functionDefinition.name);
	vector<Statement> result;
	result.push_back(FunctionalInstruction{
		loc,
		Instruction{loc, solidity::Instruction::JUMP},
		{Identifier{loc, afterFunction}}
	});
	result.push_back(Label{loc, _functionDefinition.name, {""}});

	Block body = boost::get<Block>((*this)(_functionDefinition.body).asStatement());
	Block env{loc, {}};
	vector<string> arguments(1, generateIdentifier("$ret"));
	copy(_functionDefinition.arguments.rbegin(), _functionDefinition.arguments.rend(), back_inserter(arguments));
	env.statements.push_back(Label{
		loc,
		generateIdentifier("$" + _functionDefinition.name + "_start"),
		arguments
	});
	for (auto const& r: _functionDefinition.returns | boost::adaptors::reversed)
		env.statements.push_back(VariableDeclaration{loc, r, make_shared<Statement>(Literal{loc, true, "0"})});
	env.statements.push_back(std::move(body));

	size_t const args = _functionDefinition.arguments.size();
	size_t const rets = _functionDefinition.returns.size();
	// Reorganise stack from
	// retpc argn ... arg1 retm ... ret1
	// to
	// retm ... ret1 retpc
	// Current layout with indices showing the target index (-1 means pop), i.e. should be
	// m -1 ... -1 0 1 ... (m-1)
	vector<int> stackTargetPos(1 + args + rets, -1);
	stackTargetPos[0] = rets;
	for (size_t i = 0; i < rets; ++i)
		stackTargetPos[1 + args + i] = i;

	while (stackTargetPos.back() != int(stackTargetPos.size() - 1))
		if (stackTargetPos.back() < 0)
		{
			env.statements.push_back(Instruction{loc, solidity::Instruction::POP});
			stackTargetPos.pop_back();
		}
		else
		{
			int sw = stackTargetPos.size() - stackTargetPos.back() - 1;
			assertThrow(1 <= sw && sw <= 16, DesugaringError, "Invalid swap / too many function arguments.");

			env.statements.push_back(Instruction{loc, solidity::swapInstruction(sw)});
			swap(stackTargetPos[stackTargetPos.back()], stackTargetPos.back());
		}
	for (size_t i = 0; i < stackTargetPos.size(); ++i)
		assertThrow(stackTargetPos[i] == int(i), DesugaringError, "Invalid stack reshuffling.");
	env.statements.push_back(Instruction{loc, solidity::Instruction::JUMP});

	result.push_back(std::move(env));
	result.push_back(Label{loc, afterFunction, {}});

	return {std::move(result)};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionCall const& _functionCall)
{
	auto loc = _functionCall.location;
	string retlabel = generateIdentifier("$returnFrom_" + _functionCall.functionName.name);
	vector<assembly::Statement> ret;
	ret.push_back(Identifier{loc, retlabel});
	for (auto const& arg: _functionCall.arguments | boost::adaptors::reversed)
		boost::apply_visitor(*this, arg).moveAppend(ret);
	ret.push_back(FunctionalInstruction{loc, Instruction{loc, solidity::Instruction::JUMP}, {_functionCall.functionName}});
	Scope::Identifier const* function = m_currentScope->lookup(_functionCall.functionName.name);
	solAssert(function, "");
	size_t arguments = boost::get<Scope::Function>(*function).arguments;
	size_t returns = boost::get<Scope::Function>(*function).returns;
	ret.push_back(Label{loc, retlabel, {boost::lexical_cast<string>(int(arguments) - int(returns) - 1)}});

	return {std::move(ret)};
}

ASTNodeReplacement AsmDesugar::operator()(Block const& _block)
{
	auto previousScope = m_currentScope;
	m_currentScope = m_scopes.at(&_block).get();
	Block block{_block.location, {}};
	for (auto const& statement: _block.statements)
		boost::apply_visitor(*this, statement).moveAppend(block.statements);
	m_currentScope = previousScope;
	return {block};
}

string AsmDesugar::generateIdentifier(string const& _hint, Scope const* _scope)
{
	if (_scope == nullptr)
		_scope = m_currentScope;
	for (size_t counter = 0; counter < 100000; ++counter)
	{
		string identifier = _hint + (counter > 0 ? "_" + boost::lexical_cast<string>(counter) : "");

		if (
			!m_generatedIdentifiers.count(identifier) &&
			!_scope->exists(identifier) &&
			!_scope->existsIncludingSubscopes(identifier)
		)
		{
			m_generatedIdentifiers.insert(identifier);
			return identifier;
		}
	}
	solAssert(false, "Unable to find suitable new identifier.");
}
