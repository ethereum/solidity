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

#include <libsolidity/experimental/analysis/DebugWarner.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeInference.h>
#include <libsolidity/experimental/ast/TypeSystemHelper.h>

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
		Type type = *typeInferenceAnnotation.type;
		Sort sort = m_analysis.typeSystem().env().sort(type);
		std::string sortString;
		if (sort.classes.size() != 1 || *sort.classes.begin() != m_analysis.typeSystem().primitiveClass(PrimitiveClass::Type))
			sortString = ":" + TypeSystemHelpers{m_analysis.typeSystem()}.sortToString(m_analysis.typeSystem().env().sort(type));
		m_errorReporter.info(
			4164_error,
			_node.location(),
			"Inferred type: " + TypeEnvironmentHelpers{m_analysis.typeSystem().env()}.typeToString(type) + sortString
		);
	}
	return true;
}
