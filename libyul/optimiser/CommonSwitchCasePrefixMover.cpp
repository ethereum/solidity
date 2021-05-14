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

#include <libyul/optimiser/CommonSwitchCasePrefixMover.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/AST.h>
#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/iterator.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>
#include <range/v3/action/transform.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

namespace
{
class IdentifierReplacer: public ASTCopier
{
public:
	IdentifierReplacer(map<YulString, YulString> _identifierMap): m_identifierMap(move(_identifierMap)) {}
	using ASTCopier::operator();
protected:
	YulString translateIdentifier(YulString _name) override
	{
		if (YulString const* value = util::valueOrNullptr(m_identifierMap, _name))
			return *value;
		else
			return _name;
	}
private:
	map<YulString, YulString> m_identifierMap;
};
}

void CommonSwitchCasePrefixMover::run(OptimiserStepContext&, Block& _ast)
{
	CommonSwitchCasePrefixMover{}(_ast);
}

namespace
{

template<typename Container>
auto eraseFirst(Container& _container) { return _container.erase(begin(_container)); }

optional<Statement> tryExtractFirstStatement(Identifier const* _switchExpressionAsIdentifier, vector<Case>& _cases)
{
	yulAssert(!_cases.empty() && !_cases.front().body.statements.empty(), "");
	Statement& referenceStatement = _cases.front().body.statements.front();

	if (_switchExpressionAsIdentifier)
	{
		Assignments assignments;
		visit(assignments, referenceStatement);
		if (assignments.names().count(_switchExpressionAsIdentifier->name))
			return nullopt;
	}

	if (!ranges::all_of(
		_cases | ranges::cpp20::views::drop(1),
		[&](Case& _case) {
			return !_case.body.statements.empty() &&
				SyntacticallyEqual{}(referenceStatement, _case.body.statements.front());
		}
	))
		return nullopt;

	if (VariableDeclaration const* referenceVarDecl = get_if<VariableDeclaration>(&referenceStatement))
	{
		for (Case& switchCase: _cases | ranges::cpp20::views::drop(1))
		{
			VariableDeclaration* varDecl = std::get_if<VariableDeclaration>(&switchCase.body.statements.front());
			yulAssert(varDecl, "");
			yulAssert(varDecl->variables.size() == referenceVarDecl->variables.size(), "");

			static constexpr auto nameFromTypedName = [](TypedName const& _name) { return _name.name; };
			IdentifierReplacer replacer{ranges::views::zip(
				varDecl->variables | ranges::views::transform(nameFromTypedName),
				referenceVarDecl->variables | ranges::views::transform(nameFromTypedName)
			) | ranges::to<std::map<YulString, YulString>>};
			vector<Statement> newBody;
			for (auto const& stmt: switchCase.body.statements | ranges::cpp20::views::drop(1))
				newBody.emplace_back(replacer.translate(stmt));
			switchCase.body.statements = move(newBody);
		}
		Statement result = move(referenceStatement);
		eraseFirst(_cases.front().body.statements);
		return result;
	}
	else
	{
		yulAssert(!holds_alternative<FunctionDefinition>(referenceStatement), "FunctionHoister is required.");
		Statement result = move(referenceStatement);
		for (Case& switchCase: _cases)
			eraseFirst(switchCase.body.statements);
		return result;
	}
}

}

void CommonSwitchCasePrefixMover::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> optional<vector<Statement>>
		{
			visit(_s);
			if (Switch* switchStatement = get_if<Switch>(&_s))
			{
				yulAssert(!switchStatement->cases.empty(), "");

				Identifier const* identifier = std::get_if<Identifier>(switchStatement->expression.get());
				if (!identifier && !holds_alternative<Literal>(*switchStatement->expression.get()))
					return nullopt;

				// We need to be able to tell how the default case behaves.
				if (switchStatement->cases.back().value != nullptr)
					return nullopt;

				vector<Statement> result;
				while (!switchStatement->cases.front().body.statements.empty())
					if (auto extractedStatement = tryExtractFirstStatement(identifier, switchStatement->cases))
						result.emplace_back(move(*extractedStatement));
					else
						break;

				if (!result.empty())
				{
					result.emplace_back(move(_s));
					return result;
				}
			}
			return nullopt;
		}
	);
}
