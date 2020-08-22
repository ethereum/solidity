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

#include <test/tools/ossfuzz/SolRegexpMutator.h>

#include <iostream>

using namespace solidity::test::fuzzer;
using namespace std;

size_t SRM::mutateInPlace(uint8_t* _data, size_t _size, size_t _maxSize, unsigned _ruleIdx)
{
	unsigned Idx = _ruleIdx % RegexpRules.size();
	string mutant = regex_replace(
		string(_data, _data + _size),
		RegexpRules[Idx].first,
		RegexpRules[Idx].second,
		regex_constants::format_first_only
	);

	if (mutant.size() < _maxSize)
	{
		mempcpy(_data, mutant.data(), mutant.size());
		return mutant.size();
	}
	else
		return 0;
}

