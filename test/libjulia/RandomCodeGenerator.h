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

#pragma once

#include <libjulia/ASTDataForward.h>
#include <libsolidity/inlineasm/AsmData.h>

#include <libdevcore/CommonData.h>

#include <functional>
#include <memory>
#include <random>

namespace dev
{
namespace julia
{
namespace test
{

class RandomCodeGenerator
{
public:
	RandomCodeGenerator(uint64_t _seed): m_rand(_seed)
	{
	}

	Block generate()
	{
		return generateBlock(s_maxBlockDepth);
	}

private:
	Block generateBlock(int _blockDepth);
	VariableDeclaration generateVariableDeclaration();
	FunctionDefinition generateFunctionDefinition(int _blockDepth);
	If generateIf(int _blockDepth);
	Switch generateSwitch(int _blockDepth);
	ForLoop generateForLoop(int _blockDepth);
	Assignment generateAssignment();
	ExpressionStatement generateExpressionStatement();

	Expression generateExpression(size_t _returnValues, int _expressionDepth);
	FunctionalInstruction generateFunctionalInstruction(size_t _returnValues, int _expressionDepth);
	FunctionCall generateFunctionCall(size_t _returnValues, int _expressionDepth);
	Identifier generateIdentifier();
	Literal generateLiteral();

	size_t randomInRange(size_t _min, size_t _max);
	/// @returns a random number between 0 and _weights.size() - 1, where i has
	/// probability _weights[i] / sum of weights
	size_t weightedRandom(std::vector<size_t> const& _weights);
	bool probabilityPercentage(size_t _percentage);
	std::vector<std::string> randomlyChooseFrom(std::vector<std::string> const& _names, size_t _amount);

	std::vector<TypedName> generateAndRegisterVariables(size_t _amount);

	std::vector<std::string> generateNonClashingNames(size_t _amount);
	std::string generateName();

	static int const s_maxStatementsPerBlock = 15;
	static int const s_maxBlockDepth = 10;
	static int const s_maxExpressionDepth = 4;

	std::mt19937_64 m_rand;

	/// Variables visible at the current point
	std::vector<std::string> m_variables;
	/// Functions visible at the current point
	std::map<std::string, std::pair<size_t, size_t>> m_functions;
};

}
}
}
