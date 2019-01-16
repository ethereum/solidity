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
#include <libyul/AsmDataForward.h>

namespace yul
{

/**
 * Optimiser component that detects syntactically equivalent functions.
 *
 * Prerequisite: Disambiguator
 */
class EquivalentFunctionDetector: public ASTWalker
{
public:
	static std::map<YulString, FunctionDefinition const*> run(Block& _block)
	{
		EquivalentFunctionDetector detector{};
		detector(_block);
		return std::move(detector.m_duplicates);
	}

	using ASTWalker::operator();
	void operator()(FunctionDefinition const& _fun) override;

private:
	EquivalentFunctionDetector() = default;
	/**
	 * Fast heuristic to detect distinct, resp. potentially equal functions.
	 *
	 * Defines a partial order on function definitions. If two functions
	 * are comparable (one is "less" than the other), they are distinct.
	 * If not (neither is "less" than the other), they are *potentially* equal.
	 */
	class RoughHeuristic
	{
	public:
		RoughHeuristic(FunctionDefinition const& _fun): m_fun(_fun) {}
		bool operator<(RoughHeuristic const& _rhs) const;
	private:
		std::size_t codeSize() const;
		FunctionDefinition const& m_fun;
		mutable boost::optional<std::size_t> m_codeSize;
		// In case the heuristic doesn't turn out to be good enough, we might want to define a hash function for code blocks.
	};
	std::map<RoughHeuristic, std::vector<FunctionDefinition const*>> m_candidates;
	std::map<YulString, FunctionDefinition const*> m_duplicates;
};


}
