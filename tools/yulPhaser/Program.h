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

#include <libyul/optimiser/NameDispenser.h>
#include <libyul/AsmData.h>

#include <liblangutil/Exceptions.h>

#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace solidity::langutil
{

class CharStream;

}

namespace solidity::yul
{

struct AsmAnalysisInfo;
struct Dialect;

}

namespace std
{

std::ostream& operator<<(std::ostream& _outputStream, solidity::langutil::ErrorList const& _errors);

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
	Program(Program const& program);
	Program(Program&& program):
		m_ast(std::move(program.m_ast)),
		m_dialect{program.m_dialect},
		m_nameDispenser(std::move(program.m_nameDispenser))
	{}
	Program operator=(Program const& program) = delete;
	Program operator=(Program&& program) = delete;

	static std::variant<Program, langutil::ErrorList> load(langutil::CharStream& _sourceCode);
	void optimise(std::vector<std::string> const& _optimisationSteps);

	size_t codeSize() const { return computeCodeSize(*m_ast); }
	yul::Block const& ast() const { return *m_ast; }

	friend std::ostream& operator<<(std::ostream& _stream, Program const& _program);
	std::string toJson() const;

private:
	Program(
		yul::Dialect const& _dialect,
		std::unique_ptr<yul::Block> _ast
	):
		m_ast(std::move(_ast)),
		m_dialect{_dialect},
		m_nameDispenser(_dialect, *m_ast, {})
	{}

	static std::variant<std::unique_ptr<yul::Block>, langutil::ErrorList> parseObject(
		yul::Dialect const& _dialect,
		langutil::CharStream _source
	);
	static std::variant<std::unique_ptr<yul::AsmAnalysisInfo>, langutil::ErrorList> analyzeAST(
		yul::Dialect const& _dialect,
		yul::Block const& _ast
	);
	static std::unique_ptr<yul::Block> disambiguateAST(
		yul::Dialect const& _dialect,
		yul::Block const& _ast,
		yul::AsmAnalysisInfo const& _analysisInfo
	);
	static std::unique_ptr<yul::Block> applyOptimisationSteps(
		yul::Dialect const& _dialect,
		yul::NameDispenser& _nameDispenser,
		std::unique_ptr<yul::Block> _ast,
		std::vector<std::string> const& _optimisationSteps
	);
	static size_t computeCodeSize(yul::Block const& _ast);

	std::unique_ptr<yul::Block> m_ast;
	yul::Dialect const& m_dialect;
	yul::NameDispenser m_nameDispenser;
};

}
