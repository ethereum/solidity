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
#include <libsolidity/parsing/DocStringParser.h>
#include <liblangutil/ErrorReporter.h>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

void copyMissingTags(StructurallyDocumentedAnnotation& _target, set<CallableDeclaration const*> const& _baseFunctions)
{
	if (_baseFunctions.size() != 1)
		return;

	auto& sourceDoc = dynamic_cast<StructurallyDocumentedAnnotation const&>((*_baseFunctions.begin())->annotation());

	set<string> existingTags;

	for (auto const& iterator: _target.docTags)
		existingTags.insert(iterator.first);

	for (auto const& [tag, content]: sourceDoc.docTags)
		if (tag != "inheritdoc" && !existingTags.count(tag))
			_target.docTags.emplace(tag, content);
}

CallableDeclaration const* findBaseCallable(set<CallableDeclaration const*> const& _baseFunctions, int64_t _contractId)
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
		handleCallable(_function, _function, _function.annotation());
	return true;
}

bool DocStringAnalyser::visit(VariableDeclaration const& _variable)
{
	if (!_variable.isStateVariable() && !_variable.isFileLevelVariable())
		return false;

	if (CallableDeclaration const* baseFunction = resolveInheritDoc(_variable.annotation().baseFunctions, _variable, _variable.annotation()))
		copyMissingTags(_variable.annotation(), {baseFunction});
	else if (_variable.annotation().docTags.empty())
		copyMissingTags(_variable.annotation(), _variable.annotation().baseFunctions);

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

void DocStringAnalyser::handleCallable(
	CallableDeclaration const& _callable,
	StructurallyDocumented const& _node,
	StructurallyDocumentedAnnotation& _annotation
)
{
	if (CallableDeclaration const* baseFunction = resolveInheritDoc(_callable.annotation().baseFunctions, _node, _annotation))
		copyMissingTags(_annotation, {baseFunction});
	else if (
		_annotation.docTags.empty() &&
		_callable.annotation().baseFunctions.size() == 1 &&
		parameterNamesEqual(_callable, **_callable.annotation().baseFunctions.begin())
	)
		copyMissingTags(_annotation, _callable.annotation().baseFunctions);
}

CallableDeclaration const* DocStringAnalyser::resolveInheritDoc(
	set<CallableDeclaration const*> const& _baseFuncs,
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
