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

#include <test/tools/ossfuzz/cdclsolver/FuzzerSolverInterface.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

using namespace solidity::test::fuzzer::cdclsolver;
using namespace std;

using Constraint = pair<bool, vector<int>>;
using Constraints = vector<Constraint>;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

namespace
{
#ifdef DEBUG
void printConstraints(Constraints _constraints)
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

bool validInput(string const& _input)
{
	return all_of(
		_input.begin(),
		_input.end(),
		[](unsigned char _c) { return isdigit(_c) || (_c == ',') || (_c == '-') || (_c == '\n'); }
	);
}

optional<Constraints> parseConstraints(istringstream& _input)
{
	Constraints constraints;
	for (string line; getline(_input, line); )
	{
		istringstream lineStream;
		lineStream.str(line);
		Constraint constraint;
		bool first = true;
		for (string field; getline(lineStream, field, ','); )
		{
			int val = 0;
			try
			{
				val = stoi(field);
			}
			// Fuzzer can sometimes supply invalid input to stoi that needs to be
			// rejected.
			catch (invalid_argument const&)
			{
				return nullopt;
			}
			if (first)
			{
				constraint.first = static_cast<bool>(val);
				first = false;
			}
			else
				constraint.second.emplace_back(val);
		}
		constraints.emplace_back(constraint);
	}
	// Zero input constraints is an invalid input
	if (constraints.size() < 1)
		return nullopt;
	// Incomplete constraints are invalid
	for (auto c: constraints)
		if (c.second.empty())
			return nullopt;
	return constraints;
}
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	istringstream input;
	input.str(string(reinterpret_cast<char const*>(_data), _size));
	if (validInput(input.str()))
	{
		// Parse CSV input
		auto constraints = parseConstraints(input);
		if (constraints.has_value())
		{
			FuzzerSolverInterface solverWithModels(/*supportModels=*/true);
			if (!solverWithModels.differentialCheck(constraints.value()))
			{
				cout << solverWithModels.m_lpResult << endl;
				cout << solverWithModels.m_z3Result << endl;
				solAssert(false, "LP result did not match with z3 result.");
			}
		}
	}
	return 0;
}
