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
/** @file StringUtils.h
 * @author Balajiganapathi S <balajiganapathi.s@gmail.com>
 * @date 2017
 *
 * String routines
 */

#include "StringUtils.h"
#include <algorithm>
#include <string>
#include <vector>

using namespace std;
using namespace dev;

namespace dev
{

bool stringWithinDistance(string const& _name1, string const& _name2, size_t _maxDistance)
{
	if (_name1 == _name2)
		return true;

	size_t n1 = _name1.size(), n2 = _name2.size();
	vector<vector<size_t>> dp(n1 + 1, vector<size_t>(n2 + 1));

	// In this dp formulation of Damerau–Levenshtein distance we are assuming that the strings are 1-based to make base case storage easier.
	// So index accesser to _name1 and _name2 have to be adjusted accordingly
	for (size_t i1 = 0; i1 <= n1; ++i1)
	{
		for (size_t i2 = 0; i2 <= n2; ++i2)
		{
			if (min(i1, i2) == 0)
				// Base case
				dp[i1][i2] = max(i1, i2);
			else
			{
				dp[i1][i2] = min(dp[i1 - 1][i2] + 1, dp[i1][i2 - 1] + 1);
				// Deletion and insertion
				if (_name1[i1 - 1] == _name2[i2 - 1])
					// Same chars, can skip
					dp[i1][i2] = min(dp[i1][i2], dp[i1 - 1][i2 - 1]);
				else
					// Different chars so try substitution
					dp[i1][i2] = min(dp[i1][i2], dp[i1 - 1][i2 - 1] + 1);

				if (i1 > 1 && i2 > 1 && _name1[i1 - 1] == _name2[i2 - 2] && _name1[i1 - 2] == _name2[i2 - 1])
					// Try transposing
					dp[i1][i2] = min(dp[i1][i2], dp[i1 - 2][i2 - 2] + 1);
			}
		}
	}

	size_t distance = dp[n1][n2];

	// if distance is not greater than _maxDistance, and distance is strictly less than length of both names,
	// they can be considered similar this is to avoid irrelevant suggestions
	return distance <=  _maxDistance && distance < n1 && distance < n2;
}

}
