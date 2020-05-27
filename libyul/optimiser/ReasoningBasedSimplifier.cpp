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

#include <libyul/optimiser/ReasoningBasedSimplifier.h>

#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libyul/Dialect.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libsmtutil/Z3Interface.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

#include <utility>
#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::smtutil;

void ReasoningBasedSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	set<YulString> ssaVars = SSAValueTracker::ssaVariables(_ast);
	ReasoningBasedSimplifier{_context.dialect, ssaVars}(_ast);
}

void ReasoningBasedSimplifier::operator()(VariableDeclaration& _varDecl)
{
	if (_varDecl.variables.size() != 1 || !_varDecl.value)
		return;
	YulString varName = _varDecl.variables.front().name;
	if (!m_ssaVariables.count(varName))
		return;
	m_variables.insert({varName, m_solver->newVariable("yul_" + varName.str(), SortProvider::uintSort)});
	m_solver->addAssertion(m_variables.at(varName) == encodeExpression(*_varDecl.value));
}

void ReasoningBasedSimplifier::operator()(If& _if)
{
	smtutil::Expression condition = encodeExpression(*_if.condition);
	m_solver->push();
	m_solver->addAssertion(condition == size_t(0));
	CheckResult result = m_solver->check({}).first;
	m_solver->pop();
	if (result == CheckResult::UNSATISFIABLE)
		_if.condition = make_unique<yul::Expression>(Literal{locationOf(*_if.condition), LiteralKind::Number, "1"_yulstring, {}});

	m_solver->push();
	m_solver->addAssertion(condition != size_t(0));
	CheckResult result2 = m_solver->check({}).first;
	m_solver->pop();
	if (result2 == CheckResult::UNSATISFIABLE)
		// TODO we could actually skip the body is this case
		_if.condition = make_unique<yul::Expression>(Literal{locationOf(*_if.condition), LiteralKind::Number, "0"_yulstring, {}});

	m_solver->push();
	m_solver->addAssertion(condition != 0);

	ASTModifier::operator()(_if.body);

	m_solver->pop();
}

ReasoningBasedSimplifier::ReasoningBasedSimplifier(
	Dialect const& _dialect,
	set<YulString> const& _ssaVariables
):
	m_dialect(_dialect),
	m_ssaVariables(_ssaVariables)
{
	m_solver = make_unique<smtutil::Z3Interface>();
}

smtutil::Expression ReasoningBasedSimplifier::encodeExpression(Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (auto const* builtin = dialect->builtin(_functionCall.functionName.name))
					if (builtin->instruction)
						return encodeBuiltin(*builtin->instruction, _functionCall.arguments);
			return newVariable();
		},
		[&](Identifier const& _identifier)
		{
			if (
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				return m_variables.at(_identifier.name);
			else
				return newVariable();
		},
		[&](Literal const& _literal)
		{
			return smtutil::Expression(valueOfLiteral(_literal));
		}
	}, _expression);
}

smtutil::Expression ReasoningBasedSimplifier::encodeBuiltin(
	evmasm::Instruction _instruction,
	vector<Expression> const& _arguments
)
{
	vector<smtutil::Expression> arguments = applyMap(
		_arguments,
		[this](Expression const& _expr) { return encodeExpression(_expr); }
	);
	switch (_instruction)
	{
	case evmasm::Instruction::ADD:
		return (arguments.at(0) + arguments.at(1)) % smtutil::Expression(bigint(1) << 256);
	case evmasm::Instruction::MUL:
		return (arguments.at(0) * arguments.at(1)) % smtutil::Expression(bigint(1) << 256);
	case evmasm::Instruction::SUB:
		return (arguments.at(0) - arguments.at(1)) % smtutil::Expression(bigint(1) << 256);
	case evmasm::Instruction::DIV:
		return smtutil::Expression::ite(
			arguments.at(1) == 0,
			0,
			(arguments.at(0) / arguments.at(1)) % smtutil::Expression(bigint(1) << 256)
		);
	// TODO SDIV
	case evmasm::Instruction::MOD:
		return smtutil::Expression::ite(
			arguments.at(1) == 0,
			0,
			arguments.at(0) % arguments.at(1)
		);
	// TODO SMOD
	case evmasm::Instruction::LT:
		return smtutil::Expression::ite(arguments.at(0) < arguments.at(1), 1, 0);
	case evmasm::Instruction::GT:
		return smtutil::Expression::ite(arguments.at(0) > arguments.at(1), 1, 0);
	case evmasm::Instruction::EQ:
		return smtutil::Expression::ite(arguments.at(0) == arguments.at(1), 1, 0);
	case evmasm::Instruction::ISZERO:
		return smtutil::Expression::ite(arguments.at(0) == 0, 1, 0);
	case evmasm::Instruction::AND:
		return bv2int(int2bv(arguments.at(0)) & int2bv(arguments.at(1)));
	case evmasm::Instruction::OR:
		return bv2int(int2bv(arguments.at(0)) | int2bv(arguments.at(1)));
	case evmasm::Instruction::NOT:
		return bv2int(~int2bv(arguments.at(0)));
	case evmasm::Instruction::SHL:
		// TODO Second conversion needed?
		return bv2int(int2bv(arguments.at(0)) << int2bv(arguments.at(1)));
	default:
		break;
	}
	return newVariable();
}

smtutil::Expression ReasoningBasedSimplifier::int2bv(smtutil::Expression _arg)
{
	return smtutil::Expression::int2bv(std::move(_arg), 256);
}

smtutil::Expression ReasoningBasedSimplifier::bv2int(smtutil::Expression _arg)
{
	return smtutil::Expression::bv2int(std::move(_arg));
}

smtutil::Expression ReasoningBasedSimplifier::newVariable()
{
	return m_solver->newVariable(uniqueName(), SortProvider::uintSort);
}

string ReasoningBasedSimplifier::uniqueName()
{
	return "expr_" + to_string(m_varCounter++);
}
