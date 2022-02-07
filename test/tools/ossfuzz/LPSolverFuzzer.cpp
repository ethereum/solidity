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

#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

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

	// Debug
	for (auto& i: constraints)
	{
		cout << (i.first ? "=" : "<=");
		for (auto& j: i.second)
			cout << "," << j;
		cout << endl;
	}

	// TODO: Invoke LP solver and Z3 on constraints provided by fuzzer interface,
	// comparing their outcomes.
	return 0;
}
