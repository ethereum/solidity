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
 * Optimiser component that renames identifiers to free up certain names.
 */

#include <libyul/optimiser/NameDisplacer.h>

#include <libyul/AsmData.h>


using namespace std;
using namespace dev;
using namespace yul;

void NameDisplacer::operator()(Identifier& _identifier)
{
	checkAndReplace(_identifier.name);
}

void NameDisplacer::operator()(VariableDeclaration& _varDecl)
{
	for (TypedName& var: _varDecl.variables)
		checkAndReplaceNew(var.name);

	ASTModifier::operator()(_varDecl);
}

void NameDisplacer::operator()(FunctionDefinition& _function)
{
	// Should have been done in the block already.
	yulAssert(!m_namesToFree.count(_function.name), "");

	for (auto& param: _function.parameters)
		checkAndReplaceNew(param.name);
	for (auto& retVar: _function.returnVariables)
		checkAndReplaceNew(retVar.name);

	ASTModifier::operator()(_function);
}

void NameDisplacer::operator()(FunctionCall& _funCall)
{
	checkAndReplace(_funCall.functionName.name);
	ASTModifier::operator()(_funCall);
}

void NameDisplacer::operator()(Block& _block)
{
	// First replace all the names of function definitions
	// because of scoping.
	for (auto& st: _block.statements)
		if (st.type() == typeid(FunctionDefinition))
			checkAndReplaceNew(boost::get<FunctionDefinition>(st).name);

	ASTModifier::operator()(_block);
}

void NameDisplacer::checkAndReplaceNew(YulString& _name)
{
	yulAssert(!m_translations.count(_name), "");
	if (m_namesToFree.count(_name))
		_name = (m_translations[_name] = m_nameDispenser.newName(_name));
}

void NameDisplacer::checkAndReplace(YulString& _name) const
{
	if (m_translations.count(_name))
		_name = m_translations.at(_name);
	yulAssert(!m_namesToFree.count(_name), "");
}

