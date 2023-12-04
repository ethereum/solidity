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

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/FunctionDependencyAnalysis.h>

using namespace solidity::frontend::experimental;
using namespace solidity::util;

FunctionDependencyAnalysis::FunctionDependencyAnalysis(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter())
{
}

bool FunctionDependencyAnalysis::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool FunctionDependencyAnalysis::visit(FunctionDefinition const& _functionDefinition)
{
	solAssert(!m_currentFunction);
	m_currentFunction = &_functionDefinition;
	// Insert a function definition pointer that maps to an empty set; the pointed to set will later be
	// populated in ``endVisit(Identifier const& _identifier)`` if ``m_currentFunction`` references another.
	auto [_, inserted] = annotation().functionCallGraph.edges.try_emplace(
		m_currentFunction, std::set<FunctionDefinition const*, ASTCompareByID<FunctionDefinition>>{}
	);
	solAssert(inserted);
	return true;
}

void FunctionDependencyAnalysis::endVisit(FunctionDefinition const&)
{
	m_currentFunction = nullptr;
}

void FunctionDependencyAnalysis::endVisit(Identifier const& _identifier)
{
	auto const* callee = dynamic_cast<FunctionDefinition const*>(_identifier.annotation().referencedDeclaration);
	// Check that the identifier is within a function body and is a function, and add it to the graph
	// as an ``m_currentFunction`` -> ``callee`` edge.
	if (m_currentFunction && callee)
		addEdge(m_currentFunction, callee);
}

void FunctionDependencyAnalysis::addEdge(FunctionDefinition const* _caller, FunctionDefinition const* _callee)
{
	annotation().functionCallGraph.edges[_caller].insert(_callee);
}

FunctionDependencyAnalysis::GlobalAnnotation& FunctionDependencyAnalysis::annotation()
{
	return m_analysis.annotation<FunctionDependencyAnalysis>();
}
