// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that combines syntactically equivalent functions.
 */
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/BlockHasher.h>
#include <libyul/AsmDataForward.h>

namespace solidity::yul
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
