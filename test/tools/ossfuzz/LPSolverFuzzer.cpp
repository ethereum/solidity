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

#include <test/tools/ossfuzz/lpsolver/FuzzerSolverInterface.h>

#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

using namespace solidity::test::fuzzer::lpsolver;
using namespace std;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

namespace
{
#ifdef DEBUG
void printConstraints(vector<pair<bool, vector<int>>> _constraints)
{
	for (auto& i: _constraints)
	{
		cout << (i.first ? "=" : "<=");
		for (auto& j: i.second)
			cout << "," << j;
		cout << endl;
	}
}
#endif

bool validConstraints(vector<pair<bool, vector<int>>> _constraints)
{
	// Zero input constraints is an invalid input
	if (_constraints.size() < 1)
		return false;
	// Incomplete constraints are invalid
	for (auto c: _constraints)
		if (c.second.empty())
			return false;
	return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	// Parse CSV input
	istringstream input;
	input.str(string(reinterpret_cast<char const*>(_data), _size));

	vector<pair<bool, vector<int>>> constraints;
	for (string line; getline(input, line); )
	{
		istringstream lineStream;
		lineStream.str(line);
		pair<bool, vector<int>> constraint;
		bool first = true;
		for (string field; getline(lineStream, field, ','); )
		{
			if (first)
			{
				constraint.first = static_cast<bool>(stoi(field));
				first = false;
			}
			else
				constraint.second.emplace_back(stoi(field));
		}
		constraints.emplace_back(constraint);
	}

	if (!validConstraints(constraints))
		return 0;
	else
	{
		// TODO: Z3 on constraints provided by fuzzer interface and comparing its outcome
		// with LP solver.
		FuzzerSolverInterface solverWithoutModels(/*supportModels=*/false);
		FuzzerSolverInterface solverWithModels(/*supportModels=*/true);

		solverWithoutModels.addConstraints(constraints);
		string resultWithoutModels = solverWithoutModels.checkResult();
		solverWithModels.addConstraints(constraints);
		string resultWithModels = solverWithModels.checkResult();

		if (resultWithoutModels != resultWithModels)
		{
			cout << resultWithoutModels << endl;
			cout << resultWithModels << endl;
			solAssert(false, "LP result without models did not match with result with models.");
		}
		return 0;
	}
}
