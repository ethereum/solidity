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

#include <libsolidity/analysis/PostTypeChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/Version.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SemVerHandler.h>
#include <libdevcore/Algorithms.h>

#include <boost/range/adaptor/map.hpp>
#include <memory>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;


bool PostTypeChecker::check(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

bool PostTypeChecker::visit(ContractDefinition const&)
{
	solAssert(!m_currentConstVariable, "");
	solAssert(m_constVariableDependencies.empty(), "");
	return true;
}

void PostTypeChecker::endVisit(ContractDefinition const&)
{
	solAssert(!m_currentConstVariable, "");
	for (auto declaration: m_constVariables)
		if (auto identifier = findCycle(*declaration))
			m_errorReporter.typeError(
				declaration->location(),
				"The value of the constant " + declaration->name() +
				" has a cyclic dependency via " + identifier->name() + "."
			);

	m_constVariables.clear();
	m_constVariableDependencies.clear();
}

void PostTypeChecker::endVisit(OverrideSpecifier const& _overrideSpecifier)
{
	for (ASTPointer<UserDefinedTypeName> const& override: _overrideSpecifier.overrides())
	{
		Declaration const* decl  = override->annotation().referencedDeclaration;
		solAssert(decl, "Expected declaration to be resolved.");

		if (dynamic_cast<ContractDefinition const*>(decl))
			continue;

		TypeType const* actualTypeType = dynamic_cast<TypeType const*>(decl->type());

		m_errorReporter.typeError(
			override->location(),
			"Expected contract but got " +
			actualTypeType->actualType()->toString(true) +
			"."
		);
	}
}

bool PostTypeChecker::visit(VariableDeclaration const& _variable)
{
	solAssert(!m_currentConstVariable, "");
	if (_variable.isConstant())
	{
		m_currentConstVariable = &_variable;
		m_constVariables.push_back(&_variable);
	}
	return true;
}

void PostTypeChecker::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.isConstant())
	{
		solAssert(m_currentConstVariable == &_variable, "");
		m_currentConstVariable = nullptr;
	}
}

bool PostTypeChecker::visit(Identifier const& _identifier)
{
	if (m_currentConstVariable)
		if (auto var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
			if (var->isConstant())
				m_constVariableDependencies[m_currentConstVariable].insert(var);
	return true;
}

VariableDeclaration const* PostTypeChecker::findCycle(VariableDeclaration const& _startingFrom)
{
	auto visitor = [&](VariableDeclaration const& _variable, CycleDetector<VariableDeclaration>& _cycleDetector, size_t _depth)
	{
		if (_depth >= 256)
			m_errorReporter.fatalDeclarationError(_variable.location(), "Variable definition exhausting cyclic dependency validator.");

		// Iterating through the dependencies needs to be deterministic and thus cannot
		// depend on the memory layout.
		// Because of that, we sort by AST node id.
		vector<VariableDeclaration const*> dependencies(
			m_constVariableDependencies[&_variable].begin(),
			m_constVariableDependencies[&_variable].end()
		);
		sort(dependencies.begin(), dependencies.end(), [](VariableDeclaration const* _a, VariableDeclaration const* _b) -> bool
		{
			return _a->id() < _b->id();
		});
		for (auto v: dependencies)
			if (_cycleDetector.run(*v))
				return;
	};
	return CycleDetector<VariableDeclaration>(visitor).run(_startingFrom);
}
