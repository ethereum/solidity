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
 * @date 2018
 * Module that generates IULIA code from binary input.
 */

#include <test/libjulia/RandomCodeGenerator.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/Exceptions.h>

#include <random>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;

struct GenerationImpossible: virtual Exception {};

#define filter(x) do { if (!(x)) { \
	::boost::throw_exception(GenerationImpossible() << \
		::boost::throw_file(__FILE__) << \
		::boost::throw_line(__LINE__)); \
} } while (false)

Block RandomCodeGenerator::generateBlock(int _blockDepth)
{
	// TODO first generate functions, then permute them into the block,
	// because we can call functions that come later.

	Block block;
	// Some routines here rely on the fact that generateBlock does not throw.
	if (_blockDepth <= int(randomInRange(0, 6)))
		return block;
	RandomCodeGenerator subScope(*this);

	size_t statements = randomInRange(0, s_maxStatementsPerBlock);
	for (size_t i = 0; i < statements; i++)
	{
		try
		{
			switch (weightedRandom({4, 6, 4, 1, 1, 1, 1, 3}))
			{
			case 0:
				block.statements.emplace_back(subScope.generateExpressionStatement());
				break;
			case 1:
				block.statements.emplace_back(subScope.generateAssignment());
				break;
			case 2:
				block.statements.emplace_back(subScope.generateVariableDeclaration());
				break;
			case 3:
				block.statements.emplace_back(subScope.generateFunctionDefinition(_blockDepth - 1));
				break;
			case 4:
				block.statements.emplace_back(subScope.generateIf(_blockDepth - 1));
				break;
			case 5:
				block.statements.emplace_back(subScope.generateSwitch(_blockDepth - 1));
				break;
			case 6:
				block.statements.emplace_back(subScope.generateForLoop(_blockDepth - 1));
				break;
			case 7:
				block.statements.emplace_back(subScope.generateBlock(_blockDepth - 1));
				break;
			default:
				solAssert(false, "");
			}
		}
		catch (GenerationImpossible const&)
		{
			// Just not add a statement in this case.
		}
	}
	return block;
}

VariableDeclaration RandomCodeGenerator::generateVariableDeclaration()
{
	size_t numVars = weightedRandom({0, 7, 1, 1, 1});
	VariableDeclaration varDecl;

	if (probabilityPercentage(80))
		varDecl.value = make_shared<Expression>(generateExpression(numVars, s_maxExpressionDepth));

	varDecl.variables = generateAndRegisterVariables(numVars);

	return varDecl;
}

FunctionDefinition RandomCodeGenerator::generateFunctionDefinition(int _blockDepth)
{
	size_t numParams = weightedRandom({3, 5, 4, 3, 0, 0, 1});
	size_t numRets = weightedRandom({2, 8, 4, 3, 0, 0, 1});

	FunctionDefinition funDef;
	funDef.name = generateNonClashingNames({1}).front();
	m_functions[funDef.name] = make_pair(numParams, numRets);

	vector<string> storedVariables;
	swap(storedVariables, m_variables);

	funDef.parameters = generateAndRegisterVariables(numParams);
	funDef.returnVariables = generateAndRegisterVariables(numRets);

	if (numRets == 1 && probabilityPercentage(60))
	{
		// Generate a simple `retVal := expression` body.
		try
		{
			Assignment assignment;
			assignment.value = make_shared<Expression>(generateExpression(1, s_maxExpressionDepth));
			assignment.variableNames.push_back(Identifier{{}, funDef.returnVariables.front().name});
			funDef.body.statements.emplace_back(move(assignment));
		}
		catch (GenerationImpossible const&)
		{
		}
	}
	if (funDef.body.statements.empty())
		funDef.body = generateBlock(_blockDepth - 1);

	swap(storedVariables, m_variables);

	return funDef;
}

If RandomCodeGenerator::generateIf(int _blockDepth)
{
	filter(_blockDepth > 0);
	If ifStatement;
	ifStatement.condition = make_shared<Expression>(generateExpression(1, s_maxExpressionDepth));
	ifStatement.body = generateBlock(_blockDepth - 1);
	return ifStatement;
}

Switch RandomCodeGenerator::generateSwitch(int _blockDepth)
{
	filter(_blockDepth > 0);
	size_t numCases = weightedRandom({0, 3, 3, 2, 1});
	bool hasDefault = probabilityPercentage(30);

	Switch switchStatement;
	switchStatement.expression = make_shared<Expression>(generateExpression(1, s_maxExpressionDepth));
	set<string> usedCaseExpressions;
	for (size_t i = 0; i < numCases; ++i)
	{
		Literal caseExpression;
		do
		{
			caseExpression = generateLiteral();
		}
		while (usedCaseExpressions.count(caseExpression.value));
		usedCaseExpressions.insert(caseExpression.value);
		switchStatement.cases.emplace_back(Case{
			{},
			make_shared<Literal>(caseExpression),
			generateBlock(_blockDepth - 1)
		});
	}
	if (hasDefault)
		switchStatement.cases.emplace_back(Case{{}, nullptr, generateBlock(_blockDepth - 1)});

	return switchStatement;
}

ForLoop RandomCodeGenerator::generateForLoop(int _blockDepth)
{
	filter(_blockDepth > 0);
	ForLoop forLoop;

	// TODO Handle pre (scoping)

	// generateExpression can throw!
	forLoop.condition = make_shared<Expression>(generateExpression(1, s_maxExpressionDepth));
	forLoop.body = generateBlock(_blockDepth - 1);
	forLoop.post = generateBlock(3);

	return forLoop;
}


ExpressionStatement RandomCodeGenerator::generateExpressionStatement()
{
	ExpressionStatement st;
	st.expression = generateExpression(0, s_maxExpressionDepth);
	return st;
}

Assignment RandomCodeGenerator::generateAssignment()
{
	size_t numVars = weightedRandom({0, 7, 1, 1, 1});
	filter(numVars <= m_variables.size());

	Assignment assignment;
	for (string const& name: randomlyChooseFrom(m_variables, numVars))
		assignment.variableNames.emplace_back(Identifier{{}, name});
	assignment.value = make_shared<Expression>(generateExpression(numVars, s_maxExpressionDepth));

	return assignment;
}


Expression RandomCodeGenerator::generateExpression(
	size_t _returnValues,
	int _expressionDepth
)
{
	// FunctionalInstruction, FunctionCall, Identifier, Literal
	vector<size_t> weights;
	if (_expressionDepth <= 1)
		weights = vector<size_t>{0, 0, 1, 1};
	else if (_expressionDepth <= 3)
		weights = vector<size_t>{1, 1, 3, 2};
	else
		weights = vector<size_t>{3, 2, 1, 1};

	if (_returnValues != 1)
		weights[2] = weights[3] = 0;
	if (_returnValues > 1)
		// No instruction has more than one return value.
		weights[0] = 0;
	filter(weights != vector<size_t>(0, 4));

	for (size_t i = 0; i < 6; i++)
	{
		try
		{
			switch (weightedRandom(weights))
			{
			case 0: return generateFunctionalInstruction(_returnValues, _expressionDepth - 1);
			case 1: return generateFunctionCall(_returnValues, _expressionDepth - 1);
			case 2: return generateIdentifier();
			case 3: return generateLiteral();
			default:
				solAssert(false, "");
			}
		}
		catch (GenerationImpossible const&)
		{
			// Try again
		}
	}
	// No way to generate an expression here.
	filter(false);
}

FunctionalInstruction RandomCodeGenerator::generateFunctionalInstruction(size_t _returnValues, int _expressionDepth)
{
	static vector<size_t> weightsReturnOne;
	static vector<size_t> weightsReturnZero;
	if (weightsReturnOne.empty() || weightsReturnZero.empty())
	{
		weightsReturnOne = vector<size_t>(256, 0);
		weightsReturnZero = vector<size_t>(256, 0);
		for (auto const& w: vector<tuple<solidity::Instruction, size_t>>{
				{solidity::Instruction::ADD, 12},
				{solidity::Instruction::SUB, 8},
				{solidity::Instruction::MUL, 8},
				{solidity::Instruction::DIV, 8},
				{solidity::Instruction::SDIV, 8},
				{solidity::Instruction::MOD, 8},
				{solidity::Instruction::SMOD, 8},
				{solidity::Instruction::EXP, 8},
				{solidity::Instruction::NOT, 12},
				{solidity::Instruction::LT, 8},
				{solidity::Instruction::GT, 8},
				{solidity::Instruction::SLT, 8},
				{solidity::Instruction::SGT, 8},
				{solidity::Instruction::EQ, 8},
				{solidity::Instruction::ISZERO, 8},
				{solidity::Instruction::AND, 8},
				{solidity::Instruction::OR, 8},
				{solidity::Instruction::XOR, 8},
				{solidity::Instruction::BYTE, 6},
				{solidity::Instruction::SHL, 0}, // Disabled for now
				{solidity::Instruction::SHR, 0}, // Disabled for now
				{solidity::Instruction::ADDMOD, 8},
				{solidity::Instruction::MULMOD, 8},
				{solidity::Instruction::SIGNEXTEND, 8},
				{solidity::Instruction::KECCAK256, 6},
				{solidity::Instruction::ADDRESS, 2},
				{solidity::Instruction::BALANCE, 2},
				{solidity::Instruction::ORIGIN, 2},
				{solidity::Instruction::CALLER, 2},
				{solidity::Instruction::CALLVALUE, 2},
				{solidity::Instruction::CALLDATALOAD, 4},
				{solidity::Instruction::CALLDATASIZE, 4},
				{solidity::Instruction::CODESIZE, 4},
				{solidity::Instruction::GASPRICE, 2},
				{solidity::Instruction::EXTCODESIZE, 2},
				{solidity::Instruction::RETURNDATASIZE, 2},
				{solidity::Instruction::BLOCKHASH, 2},
				{solidity::Instruction::COINBASE, 2},
				{solidity::Instruction::TIMESTAMP, 2},
				{solidity::Instruction::NUMBER, 2},
				{solidity::Instruction::DIFFICULTY, 2},
				{solidity::Instruction::GASLIMIT, 2},
				{solidity::Instruction::MLOAD, 4},
				{solidity::Instruction::SLOAD, 4},
				{solidity::Instruction::PC, 2},
				{solidity::Instruction::MSIZE, 2},
				{solidity::Instruction::GAS, 2},
				{solidity::Instruction::CREATE, 2},
				{solidity::Instruction::CALL, 2},
				{solidity::Instruction::CALLCODE, 2},
				{solidity::Instruction::DELEGATECALL, 2},
				{solidity::Instruction::STATICCALL, 2},
				{solidity::Instruction::CREATE2, 0}, // Disabled for now
		})
			 weightsReturnOne[int(get<0>(w))] = get<1>(w);

		for (auto const& w: vector<tuple<solidity::Instruction, size_t>>{
			{solidity::Instruction::CODECOPY, 2},
			{solidity::Instruction::EXTCODECOPY, 2},
			{solidity::Instruction::RETURNDATACOPY, 2},
			{solidity::Instruction::CALLDATACOPY, 4},
			{solidity::Instruction::POP, 2},
			{solidity::Instruction::MSTORE, 20},
			{solidity::Instruction::MSTORE8, 12},
			{solidity::Instruction::SSTORE, 20},
			{solidity::Instruction::LOG0, 2},
			{solidity::Instruction::LOG1, 2},
			{solidity::Instruction::LOG2, 2},
			{solidity::Instruction::LOG3, 2},
			{solidity::Instruction::LOG4, 2},
			{solidity::Instruction::RETURN, 2},
			{solidity::Instruction::REVERT, 2},
			{solidity::Instruction::INVALID, 2},
			{solidity::Instruction::SELFDESTRUCT, 2}
		})
			weightsReturnZero[int(get<0>(w))] = get<1>(w);
	}
	filter(_returnValues <= 1);

	FunctionalInstruction instr;
	instr.instruction = solidity::Instruction(weightedRandom(
		_returnValues == 0 ?
		weightsReturnZero :
		weightsReturnOne
	));
	size_t arguments = size_t(instructionInfo(instr.instruction).args);
	for (size_t i = 0; i < arguments; ++i)
		instr.arguments.emplace_back(generateExpression(1, _expressionDepth - 1));
	return instr;
}

FunctionCall RandomCodeGenerator::generateFunctionCall(size_t _returnValues, int _expressionDepth)
{
	vector<string> candidates;
	for (auto const& f: m_functions)
		if (f.second.second == _returnValues)
			candidates.push_back(f.first);
	filter(!candidates.empty());

	FunctionCall funCall;
	funCall.functionName.name = randomlyChooseFrom(candidates, 1).front();
	size_t arguments = m_functions[funCall.functionName.name].first;
	for (size_t i = 0; i < arguments; ++i)
		funCall.arguments.emplace_back(generateExpression(1, _expressionDepth - 1));
	return funCall;
}

Identifier RandomCodeGenerator::generateIdentifier()
{
	filter(!m_variables.empty());
	return Identifier{{}, randomlyChooseFrom(m_variables, 1).front()};
}

Literal RandomCodeGenerator::generateLiteral()
{
	static vector<string> const literals{"0", "1", "2", "3", "6", "100", "110", "0xff", "0xff0000000000000000", "0xffffffff", "0x10001"};
	Literal lit;
	lit.kind = assembly::LiteralKind::Number;
	lit.value = literals[randomInRange(0, literals.size() - 1)];
	return lit;
}

size_t RandomCodeGenerator::randomInRange(size_t _min, size_t _max)
{
	return std::uniform_int_distribution<size_t>{_min, _max}(m_rand);
}

size_t RandomCodeGenerator::weightedRandom(vector<size_t> const& _weights)
{
	size_t sum = 0;
	for (size_t w: _weights)
		sum += w;
	size_t r = randomInRange(0, sum - 1);
	for (size_t i = 0; i < _weights.size(); ++i)
	{
		if (r < _weights[i])
			return i;
		r -= _weights[i];
	}
	solAssert(false, "");
	return 0;
}

bool RandomCodeGenerator::probabilityPercentage(size_t _percentage)
{
	return randomInRange(0, 99) < _percentage;
}

vector<string> RandomCodeGenerator::randomlyChooseFrom(vector<string> const& _names, size_t _amount)
{
	vector<string> chosen;

	vector<size_t> weights(_names.size(), 1);
	for (size_t i = 0; i < _amount; ++i)
	{
		size_t index = weightedRandom(weights);
		chosen.push_back(_names[index]);
		weights[index] = 0;
	}

	return chosen;
}

vector<TypedName> RandomCodeGenerator::generateAndRegisterVariables(size_t _amount)
{
	vector<TypedName> variables;
	for (string const& name: generateNonClashingNames(_amount))
	{
		variables.emplace_back(assembly::TypedName{{}, name, string()});
		m_variables.push_back(name);
	}
	return variables;
}

vector<string> RandomCodeGenerator::generateNonClashingNames(size_t _amount)
{
	vector<string> names;
	while (names.size() < _amount)
	{
		string name = generateName();
		if (contains(names, name) || contains(m_variables, name) || m_functions.count(name))
			continue;
		names.emplace_back(move(name));
	}
	return names;
}

string RandomCodeGenerator::generateName()
{
	string name;
	size_t length = randomInRange(4, 8);
	for (size_t i = 0; i < length; ++i)
		name.push_back(randomInRange(size_t('a'), size_t('z')));
	return name;
}
