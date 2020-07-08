// SPDX-License-Identifier: GPL-3.0

#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/AsmData.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>
#include <libyul/Dialect.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void VarDeclInitializer::operator()(Block& _block)
{
	ASTModifier::operator()(_block);

	using OptionalStatements = std::optional<vector<Statement>>;
	util::GenericVisitor visitor{
		util::VisitorFallback<OptionalStatements>{},
		[this](VariableDeclaration& _varDecl) -> OptionalStatements
		{
			if (_varDecl.value)
				return {};

			if (_varDecl.variables.size() == 1)
			{
				_varDecl.value = make_unique<Expression>(m_dialect.zeroLiteralForType(_varDecl.variables.front().type));
				return {};
			}
			else
			{
				OptionalStatements ret{vector<Statement>{}};
				langutil::SourceLocation loc{std::move(_varDecl.location)};
				for (auto& var: _varDecl.variables)
				{
					unique_ptr<Expression> expr = make_unique<Expression >(m_dialect.zeroLiteralForType(var.type));
					ret->emplace_back(VariableDeclaration{loc, {std::move(var)}, std::move(expr)});
				}
				return ret;
			}
		}
	};

	util::iterateReplacing(_block.statements, [&](auto&& _statement) { return std::visit(visitor, _statement); });
}
