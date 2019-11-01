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
 * Creates an independent copy of an AST, renaming identifiers to be unique.
 */

#include <libyul/optimiser/ASTCopier.h>

#include <libyul/Exceptions.h>

#include <libyul/AsmData.h>

#include <libdevcore/Common.h>

using namespace std;
using namespace dev;
using namespace yul;

Statement ASTCopier::operator()(ExpressionStatement const& _statement)
{
	return ExpressionStatement{ _statement.location, translate(_statement.expression) };
}

Statement ASTCopier::operator()(VariableDeclaration const& _varDecl)
{
	return VariableDeclaration{
		_varDecl.location,
		translateVector(_varDecl.variables),
		translate(_varDecl.value)
	};
}

Statement ASTCopier::operator()(Assignment const& _assignment)
{
	return Assignment{
		_assignment.location,
		translateVector(_assignment.variableNames),
		translate(_assignment.value)
	};
}

Expression ASTCopier::operator()(FunctionCall const& _call)
{
	return FunctionCall{
		_call.location,
		translate(_call.functionName),
		translateVector(_call.arguments)
	};
}

Expression ASTCopier::operator()(FunctionalInstruction const& _instruction)
{
	return FunctionalInstruction{
		_instruction.location,
		_instruction.instruction,
		translateVector(_instruction.arguments)
	};
}

Expression ASTCopier::operator()(Identifier const& _identifier)
{
	return translate(_identifier);
}

Expression ASTCopier::operator()(Literal const& _literal)
{
	return translate(_literal);
}

Statement ASTCopier::operator()(If const& _if)
{
	return If{_if.location, translate(_if.condition), translate(_if.body)};
}

Statement ASTCopier::operator()(Switch const& _switch)
{
	return Switch{_switch.location, translate(_switch.expression), translateVector(_switch.cases)};
}

Statement ASTCopier::operator()(FunctionDefinition const& _function)
{
	YulString translatedName = translateIdentifier(_function.name);

	enterFunction(_function);
	ScopeGuard g([&]() { this->leaveFunction(_function); });

	return FunctionDefinition{
		_function.location,
		translatedName,
		translateVector(_function.parameters),
		translateVector(_function.returnVariables),
		translate(_function.body)
	};
}

Statement ASTCopier::operator()(ForLoop const& _forLoop)
{
	enterScope(_forLoop.pre);
	ScopeGuard g([&]() { this->leaveScope(_forLoop.pre); });

	return ForLoop{
		_forLoop.location,
		translate(_forLoop.pre),
		translate(_forLoop.condition),
		translate(_forLoop.post),
		translate(_forLoop.body)
	};
}
Statement ASTCopier::operator()(Break const& _break)
{
	return Break{ _break };
}

Statement ASTCopier::operator()(Continue const& _continue)
{
	return Continue{ _continue };
}

Statement ASTCopier::operator()(Leave const& _leave)
{
	return Leave{_leave};
}

Statement ASTCopier::operator ()(Block const& _block)
{
	return translate(_block);
}

Expression ASTCopier::translate(Expression const& _expression)
{
	return _expression.apply_visitor(static_cast<ExpressionCopier&>(*this));
}

Statement ASTCopier::translate(Statement const& _statement)
{
	return _statement.apply_visitor(static_cast<StatementCopier&>(*this));
}

Block ASTCopier::translate(Block const& _block)
{
	enterScope(_block);
	ScopeGuard g([&]() { this->leaveScope(_block); });

	return Block{_block.location, translateVector(_block.statements)};
}

Case ASTCopier::translate(Case const& _case)
{
	return Case{_case.location, translate(_case.value), translate(_case.body)};
}

Identifier ASTCopier::translate(Identifier const& _identifier)
{
	return Identifier{_identifier.location, translateIdentifier(_identifier.name)};
}

Literal ASTCopier::translate(Literal const& _literal)
{
	return _literal;
}

TypedName ASTCopier::translate(TypedName const& _typedName)
{
	return TypedName{_typedName.location, translateIdentifier(_typedName.name), _typedName.type};
}

