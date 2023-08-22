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

#include <libsolidity/experimental/analysis/DebugWarner.h>
#include <libsolidity/experimental/analysis/SyntaxRestrictor.h>
#include <libsolidity/experimental/analysis/TypeInference.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>

using namespace solidity::langutil;
using namespace solidity::frontend::experimental;

// TODO: creating all of them for all nodes up front may be wasteful, we should improve the mechanism.
struct Analysis::AnnotationContainer
{
	TypeRegistration::Annotation typeRegistrationAnnotation;
	TypeInference::Annotation typeInferenceAnnotation;
};

struct Analysis::GlobalAnnotationContainer
{
	TypeRegistration::GlobalAnnotation typeRegistrationAnnotation;
	TypeInference::GlobalAnnotation typeInferenceAnnotation;
};

template<>
TypeRegistration::Annotation& solidity::frontend::experimental::detail::AnnotationFetcher<TypeRegistration>::get(ASTNode const& _node)
{
	return analysis.annotationContainer(_node).typeRegistrationAnnotation;
}

template<>
TypeRegistration::GlobalAnnotation const& solidity::frontend::experimental::detail::ConstAnnotationFetcher<TypeRegistration>::get() const
{
	return analysis.annotationContainer().typeRegistrationAnnotation;
}


template<>
TypeRegistration::GlobalAnnotation& solidity::frontend::experimental::detail::AnnotationFetcher<TypeRegistration>::get()
{
	return analysis.annotationContainer().typeRegistrationAnnotation;
}

template<>
TypeRegistration::Annotation const& solidity::frontend::experimental::detail::ConstAnnotationFetcher<TypeRegistration>::get(ASTNode const& _node) const
{
	return analysis.annotationContainer(_node).typeRegistrationAnnotation;
}

template<>
TypeInference::Annotation& solidity::frontend::experimental::detail::AnnotationFetcher<TypeInference>::get(ASTNode const& _node)
{
	return analysis.annotationContainer(_node).typeInferenceAnnotation;
}

template<>
TypeInference::Annotation const& solidity::frontend::experimental::detail::ConstAnnotationFetcher<TypeInference>::get(ASTNode const& _node) const
{
	return analysis.annotationContainer(_node).typeInferenceAnnotation;
}

template<>
TypeInference::GlobalAnnotation const& solidity::frontend::experimental::detail::ConstAnnotationFetcher<TypeInference>::get() const
{
	return analysis.annotationContainer().typeInferenceAnnotation;
}


template<>
TypeInference::GlobalAnnotation& solidity::frontend::experimental::detail::AnnotationFetcher<TypeInference>::get()
{
	return analysis.annotationContainer().typeInferenceAnnotation;
}

Analysis::AnnotationContainer& Analysis::annotationContainer(ASTNode const& _node)
{
	solAssert(_node.id() > 0);
	size_t id = static_cast<size_t>(_node.id());
	solAssert(id <= m_maxAstId);
	return m_annotations[id];
}

Analysis::AnnotationContainer const& Analysis::annotationContainer(ASTNode const& _node) const
{
	solAssert(_node.id() > 0);
	size_t id = static_cast<size_t>(_node.id());
	solAssert(id <= m_maxAstId);
	return m_annotations[id];
}

Analysis::Analysis(langutil::ErrorReporter& _errorReporter, uint64_t _maxAstId):
	m_errorReporter(_errorReporter),
	m_maxAstId(_maxAstId),
	m_annotations(std::make_unique<AnnotationContainer[]>(static_cast<size_t>(_maxAstId + 1))),
	m_globalAnnotation(std::make_unique<GlobalAnnotationContainer>())
{
}

Analysis::~Analysis()
{}

template<size_t... Is>
std::tuple<std::integral_constant<size_t, Is>...> makeIndexTuple(std::index_sequence<Is...>) {
	return std::make_tuple( std::integral_constant<size_t, Is>{}...);
}

bool Analysis::check(std::vector<std::shared_ptr<SourceUnit const>> const& _sourceUnits)
{
	using AnalysisSteps = std::tuple<SyntaxRestrictor, TypeRegistration, TypeInference, DebugWarner>;

	return std::apply([&](auto... _indexTuple) {
		return ([&](auto&& _step) {
			for (auto source: _sourceUnits)
				if (!_step.analyze(*source))
					return false;
			return true;
		}(std::tuple_element_t<decltype(_indexTuple)::value, AnalysisSteps>{*this}) && ...);
	}, makeIndexTuple(std::make_index_sequence<std::tuple_size_v<AnalysisSteps>>{}));

/*
	{
		SyntaxRestrictor syntaxRestrictor{*this};
		for (auto source: _sourceUnits)
			if (!syntaxRestrictor.analyze(*source))
				return false;
	}

	{
		TypeRegistration typeRegistration{*this};
		for (auto source: _sourceUnits)
			if (!typeRegistration.analyze(*source))
				return false;
	}
	{
		TypeInference typeInference{*this};
		for (auto source: _sourceUnits)
			if (!typeInference.analyze(*source))
				return false;
	}
	return true;
 */
}
