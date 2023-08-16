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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Parses and analyses the doc strings.
 * Stores the parsing results in the AST annotations and reports errors.
 */

#include <libsolidity/analysis/DocStringAnalyser.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <liblangutil/ErrorReporter.h>

#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

void copyMissingTags(std::set<CallableDeclaration const*> const& _baseFunctions, StructurallyDocumentedAnnotation& _target, FunctionType const* _functionType = nullptr)
{
	// Only copy if there is exactly one direct base function.
	if (_baseFunctions.size() != 1)
		return;

	CallableDeclaration const& baseFunction = **_baseFunctions.begin();

	auto& sourceDoc = dynamic_cast<StructurallyDocumentedAnnotation const&>(baseFunction.annotation());

	for (auto it = sourceDoc.docTags.begin(); it != sourceDoc.docTags.end();)
	{
		std::string const& tag = it->first;
		// Don't copy tag "inheritdoc", custom tags or already existing tags
		if (tag == "inheritdoc" || _target.docTags.count(tag) || boost::starts_with(tag, "custom"))
		{
			it++;
			continue;
		}

		size_t n = 0;
		// Iterate over all values of the current tag (it's a multimap)
		for (auto next = sourceDoc.docTags.upper_bound(tag); it != next; it++, n++)
		{
			DocTag content = it->second;

			// Update the parameter name for @return tags
			if (_functionType && tag == "return")
			{
				size_t docParaNameEndPos = content.content.find_first_of(" \t");
				std::string const docParameterName = content.content.substr(0, docParaNameEndPos);

				if (
					_functionType->returnParameterNames().size() > n &&
					docParameterName != _functionType->returnParameterNames().at(n)
				)
				{
					bool baseHasNoName =
						baseFunction.returnParameterList() &&
						baseFunction.returnParameters().size() > n &&
						baseFunction.returnParameters().at(n)->name().empty();

					std::string paramName = _functionType->returnParameterNames().at(n);
					content.content =
						(paramName.empty() ? "" : std::move(paramName) + " ") + (
							std::string::npos == docParaNameEndPos || baseHasNoName ?
							content.content :
							content.content.substr(docParaNameEndPos + 1)
						);
				}
			}

			_target.docTags.emplace(tag, content);
		}
	}
}

CallableDeclaration const* findBaseCallable(std::set<CallableDeclaration const*> const& _baseFunctions, int64_t _contractId)
{
	for (CallableDeclaration const* baseFuncCandidate: _baseFunctions)
		if (baseFuncCandidate->annotation().contract->id() == _contractId)
			return baseFuncCandidate;
		else if (auto callable = findBaseCallable(baseFuncCandidate->annotation().baseFunctions, _contractId))
			return callable;

	return nullptr;
}

bool parameterNamesEqual(CallableDeclaration const& _a, CallableDeclaration const& _b)
{
	return boost::range::equal(_a.parameters(), _b.parameters(), [](auto const& pa, auto const& pb) { return pa->name() == pb->name(); });
}

}

bool DocStringAnalyser::analyseDocStrings(SourceUnit const& _sourceUnit)
{
	auto errorWatcher = m_errorReporter.errorWatcher();
	_sourceUnit.accept(*this);
	return errorWatcher.ok();
}

bool DocStringAnalyser::visit(FunctionDefinition const& _function)
{
	if (!_function.isConstructor())
		handleCallable(_function, _function, _function.annotation(), TypeProvider::function(_function));
	return true;
}

bool DocStringAnalyser::visit(VariableDeclaration const& _variable)
{
	if (!_variable.isStateVariable() && !_variable.isFileLevelVariable())
		return false;

	auto const* getterType = TypeProvider::function(_variable);

	if (CallableDeclaration const* baseFunction = resolveInheritDoc(_variable.annotation().baseFunctions, _variable, _variable.annotation()))
		copyMissingTags({baseFunction}, _variable.annotation(), getterType);
	else if (_variable.annotation().docTags.empty())
		copyMissingTags(_variable.annotation().baseFunctions, _variable.annotation(), getterType);

	return false;
}

bool DocStringAnalyser::visit(ModifierDefinition const& _modifier)
{
	handleCallable(_modifier, _modifier, _modifier.annotation());

	return true;
}

bool DocStringAnalyser::visit(EventDefinition const& _event)
{
	handleCallable(_event, _event, _event.annotation());

	return true;
}

bool DocStringAnalyser::visit(ErrorDefinition const& _error)
{
	handleCallable(_error, _error, _error.annotation());

	return true;
}

void DocStringAnalyser::handleCallable(
	CallableDeclaration const& _callable,
	StructurallyDocumented const& _node,
	StructurallyDocumentedAnnotation& _annotation,
	FunctionType const* _functionType
)
{
	if (CallableDeclaration const* baseFunction = resolveInheritDoc(_callable.annotation().baseFunctions, _node, _annotation))
		copyMissingTags({baseFunction}, _annotation, _functionType);
	else if (
		_annotation.docTags.empty() &&
		_callable.annotation().baseFunctions.size() == 1 &&
		parameterNamesEqual(_callable, **_callable.annotation().baseFunctions.begin())
	)
		copyMissingTags(_callable.annotation().baseFunctions, _annotation, _functionType);
}

CallableDeclaration const* DocStringAnalyser::resolveInheritDoc(
	std::set<CallableDeclaration const*> const& _baseFuncs,
	StructurallyDocumented const& _node,
	StructurallyDocumentedAnnotation& _annotation
)
{
	if (_annotation.inheritdocReference == nullptr)
		return nullptr;

	if (auto const callable = findBaseCallable(_baseFuncs, _annotation.inheritdocReference->id()))
		return callable;

	m_errorReporter.docstringParsingError(
		4682_error,
		_node.documentation()->location(),
		"Documentation tag @inheritdoc references contract \"" +
		_annotation.inheritdocReference->name() +
		"\", but the contract does not contain a function that is overridden by this function."
	);

	return nullptr;
}
