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


#include <libsolidity/experimental/analysis/TypeClassRegistration.h>

#include <libsolidity/experimental/analysis/Analysis.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeClassRegistration::TypeClassRegistration(Analysis& _analysis):
	m_analysis(_analysis),
	m_errorReporter(_analysis.errorReporter()),
	m_typeSystem(_analysis.typeSystem())
{
}

bool TypeClassRegistration::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeClassRegistration::visit(TypeClassDefinition const& _typeClassDefinition)
{
	Type typeVar = m_typeSystem.freshTypeVariable({});

	std::variant<TypeClass, std::string> typeClassOrError = m_typeSystem.declareTypeClass(
		typeVar,
		_typeClassDefinition.name(),
		&_typeClassDefinition
	);

	m_analysis.annotation<TypeClassRegistration>(_typeClassDefinition).typeClass = std::visit(
		util::GenericVisitor{
			[](TypeClass _class) -> TypeClass { return _class; },
			[&](std::string _error) -> TypeClass {
				m_errorReporter.fatalTypeError(4767_error, _typeClassDefinition.location(), _error);
				util::unreachable();
			}
		},
		typeClassOrError
	);

	return true;
}
