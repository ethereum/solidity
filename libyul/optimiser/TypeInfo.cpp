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
 * Helper class that keeps track of the types while performing optimizations.
 */

#include <libyul/optimiser/TypeInfo.h>

#include <libyul/optimiser/NameCollector.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <libsolutil/Visitor.h>

using namespace std;
using namespace solidity::yul;
using namespace solidity::util;

class TypeInfo::TypeCollector: public ASTWalker
{
public:
	explicit TypeCollector(Block const& _block)
	{
		(*this)(_block);
	}

	using ASTWalker::operator();
	void operator()(VariableDeclaration const& _varDecl) override
	{
		for (auto const& var: _varDecl.variables)
			variableTypes[var.name] = var.type;
	}
	void operator()(FunctionDefinition const& _funDef) override
	{
		ASTWalker::operator()(_funDef);

		auto& funType = functionTypes[_funDef.name];
		for (auto const arg: _funDef.parameters)
			funType.parameters.emplace_back(arg.type);
		for (auto const ret: _funDef.returnVariables)
			funType.returns.emplace_back(ret.type);
	}

	std::map<YulString, YulString> variableTypes;
	std::map<YulString, FunctionType> functionTypes;
};


TypeInfo::TypeInfo(Dialect const& _dialect, Block const& _ast):
	m_dialect(_dialect)
{
	TypeCollector types(_ast);
	m_functionTypes = std::move(types.functionTypes);
	m_variableTypes = std::move(types.variableTypes);
}

YulString TypeInfo::typeOf(Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _funCall) {
			YulString name = _funCall.functionName.name;
			vector<YulString> const* retTypes = nullptr;
			if (BuiltinFunction const* fun = m_dialect.builtin(name))
				retTypes = &fun->returns;
			else
				retTypes = &m_functionTypes.at(name).returns;
			yulAssert(retTypes && retTypes->size() == 1, "Call to typeOf for non-single-value expression.");
			return retTypes->front();
		},
		[&](Identifier const& _identifier) {
			return m_variableTypes.at(_identifier.name);
		},
		[&](Literal const& _literal) {
			return _literal.type;
		}
	}, _expression);
}
