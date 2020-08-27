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
/**
 * Antlr4 visitor that does at least one, at most both of the
 * following while keeping mutant syntactically valid:
 * - mutates existing Solidity source code
 * - generates additional Solidity source code
 */

#pragma once

#include <test/tools/ossfuzz/SolidityBaseVisitor.h>

#include <libsolutil/Whiskers.h>

#include <random>

namespace solidity::test::fuzzer {
using RandomEngine = std::mt19937_64;
using Dist = std::uniform_int_distribution<uint32_t>;

class SolidityMutator: public SolidityBaseVisitor
{
public:
	SolidityMutator(unsigned int _seed): m_rand(_seed) {}
	std::string toString()
	{
		return m_out.str();
	}

	antlrcpp::Any visitSourceUnit(SolidityParser::SourceUnitContext* _ctx) override;
	antlrcpp::Any visitPragmaDirective(SolidityParser::PragmaDirectiveContext* _ctx) override;
private:
	/// @returns either true or false with roughly the same probability
	bool coinToss()
	{
		return m_rand() % 2 == 0;
	}
	/// @returns a pseudo randomly chosen unsigned integer between one
	/// and @param _n
	uint32_t randomOneToN(uint32_t _n)
	{
		return Dist(1, _n)(m_rand);
	}

	/// Mutant stream
	std::ostringstream m_out;
	/// Random number generator
	RandomEngine m_rand;
	/// Whisker template strings for Solidity syntax
	const std::string s_pragmaDirective = R"(pragma <directive>;)";
};
}