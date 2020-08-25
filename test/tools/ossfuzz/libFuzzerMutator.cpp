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

#include "libFuzzerMutator.h"

#include <antlr4-runtime/antlr4-runtime.h>
#include <cstring>

using namespace std;
using namespace antlr4;
using namespace solidity::test::fuzzer;

size_t CustomMutator::mutation(Mutation _mut)
{
	if (MaxMutantSize <= Size || Size == 0)
		return LLVMFuzzerMutate(Data, Size,	MaxMutantSize);

	switch (_mut)
	{
	case Mutation::ANTLR:
	{
		string mutant = Visitor.toString();
		if (mutant.empty())
			return LLVMFuzzerMutate(Data, Size, MaxMutantSize);
		else
		{
			size_t mutantSize = mutant.size() >= MaxMutantSize ? MaxMutantSize - 1 : mutant.size();
			mempcpy(Data, mutant.data(), mutantSize);
			return mutantSize;
		}
	}
	case Mutation::NUMMUTATIONS:
		assert(false);
	}
}