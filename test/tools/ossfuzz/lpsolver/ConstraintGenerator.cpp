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

#include <test/tools/ossfuzz/lpsolver/ConstraintGenerator.h>

using namespace std;
using namespace solidity::test::fuzzer::lpsolver;

ConstraintGenerator::ConstraintGenerator(unsigned int _seed)
{
	prng = make_shared<RandomEngine>(_seed);
}

string ConstraintGenerator::generate()
{
	string constraint;
	for (int i = 0; i < numConstraints(); i++)
	{
		string sep;
		for (int j = 0; j < numFactors(); j++)
		{
			constraint += sep + to_string(randomInteger());
			if (sep.empty())
				sep = ",";
		}
		constraint += "\n";
	}
	return constraint;
}
