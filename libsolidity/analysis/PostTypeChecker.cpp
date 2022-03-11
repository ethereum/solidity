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

#include <libsolidity/analysis/PostTypeChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/Version.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SemVerHandler.h>
#include <libsolutil/Algorithms.h>
#include <libsolutil/FunctionSelector.h>

#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

bool PostTypeChecker::check(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return !Error::containsErrors(m_errorReporter.errors());
}

bool PostTypeChecker::finalize()
{
	for (auto& checker: m_checkers)
		checker->finalize();
	return !Error::containsErrors(m_errorReporter.errors());
}

bool PostTypeChecker::visit(ContractDefinition const& _contractDefinition)
{
	return callVisit(_contractDefinition);
}

void PostTypeChecker::endVisit(ContractDefinition const& _contractDefinition)
{
	callEndVisit(_contractDefinition);
}

void PostTypeChecker::endVisit(OverrideSpecifier const& _overrideSpecifier)
{
	callEndVisit(_overrideSpecifier);
}

bool PostTypeChecker::visit(VariableDeclaration const& _variable)
{
	return callVisit(_variable);
}

void PostTypeChecker::endVisit(VariableDeclaration const& _variable)
{
	callEndVisit(_variable);
}

void PostTypeChecker::endVisit(ErrorDefinition const& _error)
{
	callEndVisit(_error);
}

bool PostTypeChecker::visit(EmitStatement const& _emit)
{
	return callVisit(_emit);
}

void PostTypeChecker::endVisit(EmitStatement const& _emit)
{
	callEndVisit(_emit);
}

bool PostTypeChecker::visit(RevertStatement const& _revert)
{
	return callVisit(_revert);
}

void PostTypeChecker::endVisit(RevertStatement const& _revert)
{
	callEndVisit(_revert);
}

bool PostTypeChecker::visit(FunctionCall const& _functionCall)
{
	return callVisit(_functionCall);
}

bool PostTypeChecker::visit(Identifier const& _identifier)
{
	return callVisit(_identifier);
}

bool PostTypeChecker::visit(MemberAccess const& _memberAccess)
{
	return callVisit(_memberAccess);
}

bool PostTypeChecker::visit(StructDefinition const& _struct)
{
	return callVisit(_struct);
}

void PostTypeChecker::endVisit(StructDefinition const& _struct)
{
	callEndVisit(_struct);
}

bool PostTypeChecker::visit(ModifierInvocation const& _modifierInvocation)
{
	return callVisit(_modifierInvocation);
}

void PostTypeChecker::endVisit(ModifierInvocation const& _modifierInvocation)
{
	callEndVisit(_modifierInvocation);
}

namespace
{
struct ConstStateVarCircularReferenceChecker: public PostTypeChecker::Checker
{
	ConstStateVarCircularReferenceChecker(ErrorReporter& _errorReporter):
		Checker(_errorReporter) {}

	void finalize() override
	{
		solAssert(!m_currentConstVariable, "");
		for (auto declaration: m_constVariables)
			if (auto identifier = findCycle(*declaration))
				m_errorReporter.typeError(
					6161_error,
					declaration->location(),
					"The value of the constant " + declaration->name() +
					" has a cyclic dependency via " + identifier->name() + "."
				);
	}

	bool visit(ContractDefinition const&) override
	{
		solAssert(!m_currentConstVariable, "");
		return true;
	}

	bool visit(VariableDeclaration const& _variable) override
	{
		if (_variable.isConstant())
		{
			solAssert(!m_currentConstVariable, "");
			m_currentConstVariable = &_variable;
			m_constVariables.push_back(&_variable);
		}
		return true;
	}

	void endVisit(VariableDeclaration const& _variable) override
	{
		if (_variable.isConstant())
		{
			solAssert(m_currentConstVariable == &_variable, "");
			m_currentConstVariable = nullptr;
		}
	}

	bool visit(Identifier const& _identifier) override
	{
		if (m_currentConstVariable)
			if (auto var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
				if (var->isConstant())
					m_constVariableDependencies[m_currentConstVariable].insert(var);
		return true;
	}

	bool visit(MemberAccess const& _memberAccess) override
	{
		if (m_currentConstVariable)
			if (auto var = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
				if (var->isConstant())
					m_constVariableDependencies[m_currentConstVariable].insert(var);
		return true;
	}

	VariableDeclaration const* findCycle(VariableDeclaration const& _startingFrom)
	{
		auto visitor = [&](VariableDeclaration const& _variable, util::CycleDetector<VariableDeclaration>& _cycleDetector, size_t _depth)
		{
			if (_depth >= 256)
				m_errorReporter.fatalDeclarationError(7380_error, _variable.location(), "Variable definition exhausting cyclic dependency validator.");

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
		return util::CycleDetector<VariableDeclaration>(visitor).run(_startingFrom);
	}

private:
	VariableDeclaration const* m_currentConstVariable = nullptr;
	std::map<VariableDeclaration const*, std::set<VariableDeclaration const*>> m_constVariableDependencies;
	std::vector<VariableDeclaration const*> m_constVariables; ///< Required for determinism.
};

struct OverrideSpecifierChecker: public PostTypeChecker::Checker
{
	OverrideSpecifierChecker(ErrorReporter& _errorReporter):
		Checker(_errorReporter) {}

	void endVisit(OverrideSpecifier const& _overrideSpecifier) override
	{
		for (ASTPointer<IdentifierPath> const& override: _overrideSpecifier.overrides())
		{
			Declaration const* decl = override->annotation().referencedDeclaration;
			solAssert(decl, "Expected declaration to be resolved.");

			if (dynamic_cast<ContractDefinition const*>(decl))
				continue;

			auto const* typeType = dynamic_cast<TypeType const*>(decl->type());
			m_errorReporter.typeError(
				9301_error,
				override->location(),
				"Expected contract but got " +
				(typeType ? typeType->actualType() : decl->type())->toString(true) +
				"."
			);
		}
	}
};

struct ModifierContextChecker: public PostTypeChecker::Checker
{
	ModifierContextChecker(ErrorReporter& _errorReporter):
		Checker(_errorReporter) {}

	bool visit(ModifierInvocation const&) override
	{
		m_insideModifierInvocation = true;

		return true;
	}

	void endVisit(ModifierInvocation const&) override
	{
		m_insideModifierInvocation = false;
	}

	bool visit(Identifier const& _identifier) override
	{
		if (m_insideModifierInvocation)
			return true;

		if (ModifierType const* type = dynamic_cast<decltype(type)>(_identifier.annotation().type))
		{
			m_errorReporter.typeError(
				3112_error,
				_identifier.location(),
				"Modifier can only be referenced in function headers."
			);
		}

		return false;
	}
private:
	/// Flag indicating whether we are currently inside the invocation of a modifier
	bool m_insideModifierInvocation = false;
};

struct EventOutsideEmitErrorOutsideRevertChecker: public PostTypeChecker::Checker
{
	EventOutsideEmitErrorOutsideRevertChecker(ErrorReporter& _errorReporter):
		Checker(_errorReporter) {}

	bool visit(EmitStatement const& _emitStatement) override
	{
		m_currentStatement = &_emitStatement;
		return true;
	}

	void endVisit(EmitStatement const&) override
	{
		m_currentStatement = nullptr;
	}

	bool visit(RevertStatement const& _revertStatement) override
	{
		m_currentStatement = &_revertStatement;
		return true;
	}

	void endVisit(RevertStatement const&) override
	{
		m_currentStatement = nullptr;
	}

	bool visit(FunctionCall const& _functionCall) override
	{
		if (*_functionCall.annotation().kind == FunctionCallKind::FunctionCall)
			if (auto const* functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type))
			{
				// Check for event outside of emit statement
				if (!dynamic_cast<EmitStatement const*>(m_currentStatement) && functionType->kind() == FunctionType::Kind::Event)
					m_errorReporter.typeError(
						3132_error,
						_functionCall.location(),
						"Event invocations have to be prefixed by \"emit\"."
					);
				else if (!dynamic_cast<RevertStatement const*>(m_currentStatement) && functionType->kind() == FunctionType::Kind::Error)
					m_errorReporter.typeError(
						7757_error,
						_functionCall.location(),
						"Errors can only be used with revert statements: \"revert MyError();\"."
					);
			}
		m_currentStatement = nullptr;

		return true;
	}

private:
	Statement const* m_currentStatement = nullptr;
};

struct NoVariablesInInterfaceChecker: public PostTypeChecker::Checker
{
	NoVariablesInInterfaceChecker(ErrorReporter& _errorReporter):
		Checker(_errorReporter)
	{}

	bool visit(VariableDeclaration const& _variable) override
	{
		// Forbid any variable declarations inside interfaces unless they are part of
		// * a function's input/output parameters,
		// * or inside of a struct definition.
		if (
			m_scope && m_scope->isInterface()
			&& !_variable.isCallableOrCatchParameter()
			&& !m_insideStruct
		)
			m_errorReporter.typeError(8274_error, _variable.location(), "Variables cannot be declared in interfaces.");

		return true;
	}

	bool visit(ContractDefinition const& _contract) override
	{
		m_scope = &_contract;
		return true;
	}

	void endVisit(ContractDefinition const&) override
	{
		m_scope = nullptr;
	}

	bool visit(StructDefinition const&) override
	{
		solAssert(m_insideStruct >= 0, "");
		m_insideStruct++;
		return true;
	}

	void endVisit(StructDefinition const&) override
	{
		m_insideStruct--;
		solAssert(m_insideStruct >= 0, "");
	}
private:
	ContractDefinition const* m_scope = nullptr;
	/// Flag indicating whether we are currently inside a StructDefinition.
	int m_insideStruct = 0;
};

struct ReservedErrorSelector: public PostTypeChecker::Checker
{
	ReservedErrorSelector(ErrorReporter& _errorReporter):
		Checker(_errorReporter)
	{}

	void endVisit(ErrorDefinition const& _error) override
	{
		if (_error.name() == "Error" || _error.name() == "Panic")
			m_errorReporter.syntaxError(
				1855_error,
				_error.location(),
				"The built-in errors \"Error\" and \"Panic\" cannot be re-defined."
			);
		else
		{
			uint32_t selector = util::selectorFromSignature32(_error.functionType(true)->externalSignature());
			if (selector == 0 || ~selector == 0)
				m_errorReporter.syntaxError(
					2855_error,
					_error.location(),
					"The selector 0x" + util::toHex(toCompactBigEndian(selector, 4)) + " is reserved. Please rename the error to avoid the collision."
				);
		}
	}
};

}


PostTypeChecker::PostTypeChecker(langutil::ErrorReporter& _errorReporter): m_errorReporter(_errorReporter)
{
	m_checkers.push_back(make_shared<ConstStateVarCircularReferenceChecker>(_errorReporter));
	m_checkers.push_back(make_shared<OverrideSpecifierChecker>(_errorReporter));
	m_checkers.push_back(make_shared<ModifierContextChecker>(_errorReporter));
	m_checkers.push_back(make_shared<EventOutsideEmitErrorOutsideRevertChecker>(_errorReporter));
	m_checkers.push_back(make_shared<NoVariablesInInterfaceChecker>(_errorReporter));
	m_checkers.push_back(make_shared<ReservedErrorSelector>(_errorReporter));
}
