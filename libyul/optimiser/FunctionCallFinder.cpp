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

#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/AST.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

vector<FunctionCall*> FunctionCallFinder::run(Block& _block, YulString _functionName)
{
	FunctionCallFinder functionCallFinder(_functionName);
	functionCallFinder(_block);
	return functionCallFinder.m_calls;
}

FunctionCallFinder::FunctionCallFinder(YulString _functionName): m_functionName(_functionName) {}

void FunctionCallFinder::operator()(FunctionCall& _functionCall)
{
	ASTModifier::operator()(_functionCall);
	if (_functionCall.functionName.name == m_functionName)
		m_calls.emplace_back(&_functionCall);
}
