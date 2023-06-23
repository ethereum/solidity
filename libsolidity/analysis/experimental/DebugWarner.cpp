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

#include <libsolidity/analysis/experimental/DebugWarner.h>

#include <libsolidity/analysis/experimental/Analysis.h>
#include <libsolidity/analysis/experimental/TypeInference.h>

#include <liblangutil/Exceptions.h>

using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

DebugWarner::DebugWarner(Analysis& _analysis): m_analysis(_analysis), m_errorReporter(_analysis.errorReporter())
{}

bool DebugWarner::analyze(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return !Error::containsErrors(m_errorReporter.errors());
}

bool DebugWarner::visitNode(ASTNode const& _node)
{
	auto const& typeInferenceAnnotation = m_analysis.annotation<TypeInference>(_node);
	if (typeInferenceAnnotation.type)
	{
		m_errorReporter.info(0000_error, _node.location(), "Inferred type: " + m_analysis.typeSystem().env().typeToString(*typeInferenceAnnotation.type));
	}
	return true;
}
