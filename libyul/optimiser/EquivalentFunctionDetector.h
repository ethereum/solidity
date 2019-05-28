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
 * Optimiser component that combines syntactically equivalent functions.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/BlockHasher.h>
#include <libyul/AsmDataForward.h>

namespace yul
{

/**
 * Optimiser component that detects syntactically equivalent functions.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter
 */
class EquivalentFunctionDetector: public ASTWalker
{
public:
	static std::map<YulString, FunctionDefinition const*> run(Block& _block)
	{
		EquivalentFunctionDetector detector{BlockHasher::run(_block)};
		detector(_block);
		return std::move(detector.m_duplicates);
	}

	using ASTWalker::operator();
	void operator()(FunctionDefinition const& _fun) override;

private:
	EquivalentFunctionDetector(std::map<Block const*, uint64_t> _blockHashes): m_blockHashes(std::move(_blockHashes)) {}

	std::map<Block const*, uint64_t> m_blockHashes;
	std::map<uint64_t, std::vector<FunctionDefinition const*>> m_candidates;
	std::map<YulString, FunctionDefinition const*> m_duplicates;
};


}
