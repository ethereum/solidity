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

bool DocStringAnalyser::analyseDocStrings(SourceUnit const& _sourceUnit)
{
	m_errorOccured = false;
	_sourceUnit.accept(*this);

	return !m_errorOccured;
}

bool DocStringAnalyser::visit(ContractDefinition const& _contract)
{
	static set<string> const validTags = set<string>{"author", "title", "dev", "notice"};
	parseDocStrings(_contract, _contract.annotation(), validTags, "contracts");

	return true;
}

bool DocStringAnalyser::visit(FunctionDefinition const& _function)
{
	if (_function.isConstructor())
		handleConstructor(_function, _function, _function.annotation());
	else
		handleCallable(_function, _function, _function.annotation());
	return true;
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

void DocStringAnalyser::checkParameters(
	CallableDeclaration const& _callable,
	DocumentedAnnotation& _annotation
)
{
	set<string> validParams;
	for (auto const& p: _callable.parameters())
		validParams.insert(p->name());
	if (_callable.returnParameterList())
		for (auto const& p: _callable.returnParameterList()->parameters())
			validParams.insert(p->name());
	auto paramRange = _annotation.docTags.equal_range("param");
	for (auto i = paramRange.first; i != paramRange.second; ++i)
		if (!validParams.count(i->second.paramName))
			appendError(
				"Documented parameter \"" +
				i->second.paramName +
				"\" not found in the parameter list of the function."
			);

}

void DocStringAnalyser::handleConstructor(
	CallableDeclaration const& _callable,
	Documented const& _node,
	DocumentedAnnotation& _annotation
)
{
	static set<string> const validTags = set<string>{"author", "dev", "notice", "param"};
	parseDocStrings(_node, _annotation, validTags, "constructor");
	checkParameters(_callable, _annotation);
}

void DocStringAnalyser::handleCallable(
	CallableDeclaration const& _callable,
	Documented const& _node,
	DocumentedAnnotation& _annotation
)
{
	static set<string> const validTags = set<string>{"author", "dev", "notice", "return", "param"};
	parseDocStrings(_node, _annotation, validTags, "functions");
	checkParameters(_callable, _annotation);
}

void DocStringAnalyser::parseDocStrings(
	Documented const& _node,
	DocumentedAnnotation& _annotation,
	set<string> const& _validTags,
	string const& _nodeName
)
{
	DocStringParser parser;
	if (_node.documentation() && !_node.documentation()->empty())
	{
		if (!parser.parse(*_node.documentation(), m_errorReporter))
			m_errorOccured = true;
		_annotation.docTags = parser.tags();
	}

	size_t returnTagsVisited = 0;
	for (auto const& docTag: _annotation.docTags)
	{
		if (!_validTags.count(docTag.first))
			appendError("Documentation tag @" + docTag.first + " not valid for " + _nodeName + ".");
		else
			if (docTag.first == "return")
			{
				returnTagsVisited++;
				if (auto* function = dynamic_cast<FunctionDefinition const*>(&_node))
				{
					string content = docTag.second.content;
					string firstWord = content.substr(0, content.find_first_of(" \t"));

					if (returnTagsVisited > function->returnParameters().size())
						appendError("Documentation tag \"@" + docTag.first + " " + docTag.second.content + "\"" +
							" exceedes the number of return parameters."
						);
					else
					{
						auto parameter = function->returnParameters().at(returnTagsVisited - 1);
						if (!parameter->name().empty() && parameter->name() != firstWord)
							appendError("Documentation tag \"@" + docTag.first + " " + docTag.second.content + "\"" +
								" does not contain the name of its return parameter."
							);
					}
				}
			}
	}
}

void DocStringAnalyser::appendError(string const& _description)
{
	m_errorOccured = true;
	m_errorReporter.docstringParsingError(_description);
}
