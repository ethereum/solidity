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

#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>
#include <libyul/Dialect.h>

using namespace solidity;
using namespace solidity::yul;

void VarDeclInitializer::operator()(Block& _block)
{
	ASTModifier::operator()(_block);

	using OptionalStatements = std::optional<std::vector<Statement>>;
	util::GenericVisitor visitor{
		util::VisitorFallback<OptionalStatements>{},
		[this](VariableDeclaration& _varDecl) -> OptionalStatements
		{
			if (_varDecl.value)
				return {};

			if (_varDecl.variables.size() == 1)
			{
				_varDecl.value = std::make_unique<Expression>(m_dialect.zeroLiteral());
				return {};
			}
			else
			{
				OptionalStatements ret{std::vector<Statement>{}};
				for (auto& var: _varDecl.variables)
				{
					std::unique_ptr<Expression> expr = std::make_unique<Expression>(m_dialect.zeroLiteral());
					ret->emplace_back(VariableDeclaration{std::move(_varDecl.debugData), {std::move(var)}, std::move(expr)});
				}
				return ret;
			}
		}
	};

	util::iterateReplacing(_block.statements, [&](auto&& _statement) { return std::visit(visitor, _statement); });
}
