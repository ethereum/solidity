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
	assertThrow(!secondValid, DesugaringError, "Tried to convert pair of statements to single.");
	return std::move(node);
}

bool ASTNodeReplacement::isBlock() const
{
	assertThrow(!secondValid, DesugaringError, "Expected single statement but was pair.");
	return node.type() == typeid(assembly::Block);
}

ASTNodeReplacement::operator assembly::Statement() &&
{
	assertThrow(!secondValid, DesugaringError, "Tried to convert pair of statements to single.");
	return std::move(node);
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
		if (arguments.back().isBlock())
			haveToExplode = true;
	}
	if (haveToExplode)
	{
		assembly::Block block;
		block.location = _functionalInstruction.location;
		std::move(arguments.rbegin(), arguments.rend(), std::back_inserter(block.statements));
		block.statements.push_back(_functionalInstruction.instruction);
		return {block};
	}
	else
	{
		FunctionalInstruction result;
		result.location = _functionalInstruction.location;
		result.instruction = _functionalInstruction.instruction;
		std::move(arguments.begin(), arguments.end(), std::back_inserter(result.arguments));
		return {result};
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
	if (value.isBlock())
		return {value.asStatement(), Assignment{_functionalAssignment.location, _functionalAssignment.variableName}};
	else
	{
		FunctionalAssignment result;
		result.location = _functionalAssignment.location;
		result.variableName = _functionalAssignment.variableName;
		result.value = make_shared<Statement>(value.asStatement());
		return {result};
	}
}

ASTNodeReplacement AsmDesugar::operator()(assembly::VariableDeclaration const& _variableDeclaration)
{
	//@TODO handle special case of `let x := f()` where return label and variable introduction label can be combined.
	ASTNodeReplacement value = boost::apply_visitor(*this, *_variableDeclaration.value);
	if (value.isBlock())
		//@TODO make the name unique
		return {value.asStatement(), Label{
			_variableDeclaration.location,
			"$introduce_" + _variableDeclaration.name,
			{_variableDeclaration.name}
		}};
	else
	{
		VariableDeclaration result;
		result.location = _variableDeclaration.location;
		result.name = _variableDeclaration.name;
		result.value = make_shared<Statement>(value.asStatement());
		return {result};
	}
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionDefinition const& _functionDefinition)
{
	auto loc = _functionDefinition.location;
	//@TODO make labels unique
	Block body = boost::get<Block>((*this)(_functionDefinition.body).asStatement());
	Block env{loc, {}};
	env.statements.push_back(Label{loc, "$" + _functionDefinition.name + "_start", _functionDefinition.arguments});
	for (auto const& r: _functionDefinition.returns)
		env.statements.push_back(VariableDeclaration{loc, r, make_shared<Statement>(Literal{loc, true, "0"})});
	env.statements.push_back(std::move(body));

	size_t const args = _functionDefinition.arguments.size();
	size_t const rets = _functionDefinition.returns.size();
	// Reorganise stack from
	// retpc arg1 ... argn ret1 ... retm
	// to
	// ret1 ... retm retpc
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

	return {Label{loc, _functionDefinition.name, {}}, env};
}

ASTNodeReplacement AsmDesugar::operator()(assembly::FunctionCall const& _functionCall)
{
	//@TODO check that the number of arguments and return values match up
	//@TOOD Make the identifiers unique
	auto loc = _functionCall.location;
	string retlabel = "$funcall_" + _functionCall.functionName.name + "_return";
	Block ret{loc, {}};
	ret.statements.push_back(Identifier{loc, retlabel});
	for (auto const& arg: _functionCall.arguments | boost::adaptors::reversed)
		ret.statements.push_back(boost::apply_visitor(*this, arg).asStatement());
	ret.statements.push_back(FunctionalInstruction{loc, Instruction{loc, solidity::Instruction::JUMP}, {_functionCall.functionName}});
	//@TODO add proper stack info here
	ret.statements.push_back(Label{loc, retlabel, {}});

	return {ret};
}

ASTNodeReplacement AsmDesugar::operator()(Block const& _block)
{
	Block block{_block.location, {}};
	for (auto const& statement: _block.statements)
	{
		ASTNodeReplacement replacement = boost::apply_visitor(*this, statement);
		block.statements.push_back(std::move(replacement.node));
		if (replacement.secondValid)
			block.statements.push_back(std::move(replacement.secondNode));
	}
	return {block};
}


