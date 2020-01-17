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

#pragma once

#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace solidity::langutil
{

class CharStream;

}

namespace solidity::yul
{

struct AsmAnalysisInfo;
struct Block;
struct Dialect;
class YulString;

}

namespace solidity::phaser
{

/**
 * Class representing parsed and analysed Yul program that we can apply optimisations to.
 * The program is already disambiguated and has several prerequisite optimiser steps applied to it
 * so that the requirements of any possible step that could be later applied by the user are
 * already satisfied.
 *
 * The class allows the user to apply extra optimisations and obtain metrics and general
 * information about the resulting syntax tree.
 */
class Program
{
public:
	static Program load(std::string const& _sourcePath);
	void optimise(std::vector<std::string> const& _optimisationSteps);

	size_t codeSize() const { return computeCodeSize(*m_ast); }
	yul::Block const& ast() const { return *m_ast; }

private:
	Program(
		yul::Dialect const& _dialect,
		std::shared_ptr<yul::Block> _ast
	):
		m_ast(_ast),
		m_nameDispenser(_dialect, *_ast, s_externallyUsedIdentifiers),
		m_optimiserStepContext{_dialect, m_nameDispenser, s_externallyUsedIdentifiers}
	{}

	static langutil::CharStream loadSource(std::string const& _sourcePath);
	static std::shared_ptr<yul::Block> parseSource(
		yul::Dialect const& _dialect,
		langutil::CharStream _source
	);
	static std::unique_ptr<yul::AsmAnalysisInfo> analyzeAST(
		yul::Dialect const& _dialect,
		yul::Block const& _ast
	);
	static std::unique_ptr<yul::Block> disambiguateAST(
		yul::Dialect const& _dialect,
		yul::Block const& _ast,
		yul::AsmAnalysisInfo const& _analysisInfo
	);
	static void applyOptimisationSteps(
		yul::OptimiserStepContext& _context,
		yul::Block& _ast,
		std::vector<std::string> const& _optimisationSteps
	);
	static size_t computeCodeSize(yul::Block const& _ast);

	std::shared_ptr<yul::Block> m_ast;
	yul::NameDispenser m_nameDispenser;
	yul::OptimiserStepContext m_optimiserStepContext;

	// Always empty set of reserved identifiers. It could be a constructor parameter but I don't
	// think it would be useful in this tool. Other tools (like yulopti) have it empty too.
	// We need it to live as long as m_optimiserStepContext because it stores a reference
	// but since it's empty and read-only we can just have all objects share one static instance.
	static std::set<yul::YulString> const s_externallyUsedIdentifiers;
};

}
