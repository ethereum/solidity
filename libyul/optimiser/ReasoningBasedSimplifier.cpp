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
#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/Utilities.h>
#include <libyul/Dialect.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

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

std::optional<string> ReasoningBasedSimplifier::invalidInCurrentEnvironment()
{
	// SMTLib2 interface is always available, but we would like to have synchronous answers.
	if (smtutil::SMTPortfolio{}.solvers() <= 1)
		return string{"No SMT solvers available."};
	else
		return nullopt;
}

void ReasoningBasedSimplifier::operator()(VariableDeclaration& _varDecl)
{
	if (_varDecl.variables.size() != 1 || !_varDecl.value)
		return;
	YulString varName = _varDecl.variables.front().name;
	if (!m_ssaVariables.count(varName))
		return;
	bool const inserted = m_variables.insert({varName, m_solver->newVariable("yul_" + varName.str(), defaultSort())}).second;
	yulAssert(inserted, "");
	m_solver->addAssertion(m_variables.at(varName) == encodeExpression(*_varDecl.value));
}

void ReasoningBasedSimplifier::operator()(If& _if)
{
	if (!SideEffectsCollector{m_dialect, *_if.condition}.movable())
		return;

	smtutil::Expression condition = encodeExpression(*_if.condition);
	m_solver->push();
	m_solver->addAssertion(condition == constantValue(0));
	CheckResult result = m_solver->check({}).first;
	m_solver->pop();
	if (result == CheckResult::UNSATISFIABLE)
	{
		Literal trueCondition = m_dialect.trueLiteral();
		trueCondition.location = locationOf(*_if.condition);
		_if.condition = make_unique<yul::Expression>(move(trueCondition));
	}
	else
	{
		m_solver->push();
		m_solver->addAssertion(condition != constantValue(0));
		CheckResult result2 = m_solver->check({}).first;
		m_solver->pop();
		if (result2 == CheckResult::UNSATISFIABLE)
		{
			Literal falseCondition = m_dialect.zeroLiteralForType(m_dialect.boolType);
			falseCondition.location = locationOf(*_if.condition);
			_if.condition = make_unique<yul::Expression>(move(falseCondition));
			_if.body = yul::Block{};
			// Nothing left to be done.
			return;
		}
	}

	m_solver->push();
	m_solver->addAssertion(condition != constantValue(0));

	ASTModifier::operator()(_if.body);

	m_solver->pop();
}

ReasoningBasedSimplifier::ReasoningBasedSimplifier(
	Dialect const& _dialect,
	set<YulString> const& _ssaVariables
):
	m_dialect(_dialect),
	m_ssaVariables(_ssaVariables),
	m_solver(make_unique<smtutil::SMTPortfolio>())
{
}

smtutil::Expression ReasoningBasedSimplifier::encodeExpression(yul::Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (auto const* builtin = dialect->builtin(_functionCall.functionName.name))
					if (builtin->instruction)
						return encodeEVMBuiltin(*builtin->instruction, _functionCall.arguments);
			return newRestrictedVariable();
		},
		[&](Identifier const& _identifier)
		{
			if (
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				return m_variables.at(_identifier.name);
			else
				return newRestrictedVariable();
		},
		[&](Literal const& _literal)
		{
			return literalValue(_literal);
		}
	}, _expression);
}

smtutil::Expression ReasoningBasedSimplifier::encodeEVMBuiltin(
	evmasm::Instruction _instruction,
	vector<yul::Expression> const& _arguments
)
{
	vector<smtutil::Expression> arguments = applyMap(
		_arguments,
		[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
	);
	switch (_instruction)
	{
	case evmasm::Instruction::ADD:
		return wrap(arguments.at(0) + arguments.at(1));
	case evmasm::Instruction::MUL:
		return wrap(arguments.at(0) * arguments.at(1));
	case evmasm::Instruction::SUB:
		return wrap(arguments.at(0) - arguments.at(1));
	case evmasm::Instruction::DIV:
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			arguments.at(0) / arguments.at(1)
		);
	case evmasm::Instruction::SDIV:
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			// No `wrap()` needed here, because -2**255 / -1 results
			// in 2**255 which is "converted" to its two's complement
			// representation 2**255 in `signedToUnsigned`
			signedToUnsigned(smtutil::signedDivisionEVM(
				unsignedToSigned(arguments.at(0)),
				unsignedToSigned(arguments.at(1))
			))
		);
	case evmasm::Instruction::MOD:
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			arguments.at(0) % arguments.at(1)
		);
	case evmasm::Instruction::SMOD:
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			signedToUnsigned(signedModuloEVM(
				unsignedToSigned(arguments.at(0)),
				unsignedToSigned(arguments.at(1))
			))
		);
	case evmasm::Instruction::LT:
		return booleanValue(arguments.at(0) < arguments.at(1));
	case evmasm::Instruction::SLT:
		return booleanValue(unsignedToSigned(arguments.at(0)) < unsignedToSigned(arguments.at(1)));
	case evmasm::Instruction::GT:
		return booleanValue(arguments.at(0) > arguments.at(1));
	case evmasm::Instruction::SGT:
		return booleanValue(unsignedToSigned(arguments.at(0)) > unsignedToSigned(arguments.at(1)));
	case evmasm::Instruction::EQ:
		return booleanValue(arguments.at(0) == arguments.at(1));
	case evmasm::Instruction::ISZERO:
		return booleanValue(arguments.at(0) == constantValue(0));
	case evmasm::Instruction::AND:
		return smtutil::Expression::ite(
			(arguments.at(0) == 0 || arguments.at(0) == 1) &&
			(arguments.at(1) == 0 || arguments.at(1) == 1),
			booleanValue(arguments.at(0) == 1 && arguments.at(1) == 1),
			bv2int(int2bv(arguments.at(0)) & int2bv(arguments.at(1)))
		);
	case evmasm::Instruction::OR:
		return smtutil::Expression::ite(
			(arguments.at(0) == 0 || arguments.at(0) == 1) &&
			(arguments.at(1) == 0 || arguments.at(1) == 1),
			booleanValue(arguments.at(0) == 1 || arguments.at(1) == 1),
			bv2int(int2bv(arguments.at(0)) | int2bv(arguments.at(1)))
		);
	case evmasm::Instruction::XOR:
		return bv2int(int2bv(arguments.at(0)) ^ int2bv(arguments.at(1)));
	case evmasm::Instruction::NOT:
		return smtutil::Expression(u256(-1)) - arguments.at(0);
	case evmasm::Instruction::SHL:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			bv2int(int2bv(arguments.at(1)) << int2bv(arguments.at(0)))
		);
	case evmasm::Instruction::SHR:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			bv2int(int2bv(arguments.at(1)) >> int2bv(arguments.at(0)))
		);
	case evmasm::Instruction::SAR:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			bv2int(smtutil::Expression::ashr(int2bv(arguments.at(1)), int2bv(arguments.at(0))))
		);
	case evmasm::Instruction::ADDMOD:
		return smtutil::Expression::ite(
			arguments.at(2) == constantValue(0),
			constantValue(0),
			(arguments.at(0) + arguments.at(1)) % arguments.at(2)
		);
	case evmasm::Instruction::MULMOD:
		return smtutil::Expression::ite(
			arguments.at(2) == constantValue(0),
			constantValue(0),
			(arguments.at(0) * arguments.at(1)) % arguments.at(2)
		);
	// TODO SIGNEXTEND
	default:
		break;
	}
	return newRestrictedVariable();
}

smtutil::Expression ReasoningBasedSimplifier::int2bv(smtutil::Expression _arg) const
{
	return smtutil::Expression::int2bv(std::move(_arg), 256);
}

smtutil::Expression ReasoningBasedSimplifier::bv2int(smtutil::Expression _arg) const
{
	return smtutil::Expression::bv2int(std::move(_arg));
}

smtutil::Expression ReasoningBasedSimplifier::newVariable()
{
	return m_solver->newVariable(uniqueName(), defaultSort());
}

smtutil::Expression ReasoningBasedSimplifier::newRestrictedVariable()
{
	smtutil::Expression var = newVariable();
	m_solver->addAssertion(0 <= var && var < smtutil::Expression(bigint(1) << 256));
	return var;
}

string ReasoningBasedSimplifier::uniqueName()
{
	return "expr_" + to_string(m_varCounter++);
}

shared_ptr<Sort> ReasoningBasedSimplifier::defaultSort() const
{
	return SortProvider::intSort();
}

smtutil::Expression ReasoningBasedSimplifier::booleanValue(smtutil::Expression _value) const
{
	return smtutil::Expression::ite(_value, constantValue(1), constantValue(0));
}

smtutil::Expression ReasoningBasedSimplifier::constantValue(size_t _value) const
{
	return _value;
}

smtutil::Expression ReasoningBasedSimplifier::literalValue(Literal const& _literal) const
{
	return smtutil::Expression(valueOfLiteral(_literal));
}

smtutil::Expression ReasoningBasedSimplifier::unsignedToSigned(smtutil::Expression _value)
{
	return smtutil::Expression::ite(
		_value < smtutil::Expression(bigint(1) << 255),
		_value,
		_value - smtutil::Expression(bigint(1) << 256)
	);
}

smtutil::Expression ReasoningBasedSimplifier::signedToUnsigned(smtutil::Expression _value)
{
	return smtutil::Expression::ite(
		_value >= 0,
		_value,
		_value + smtutil::Expression(bigint(1) << 256)
	);
}

smtutil::Expression ReasoningBasedSimplifier::wrap(smtutil::Expression _value)
{
	smtutil::Expression rest = newRestrictedVariable();
	smtutil::Expression multiplier = newVariable();
	m_solver->addAssertion(_value == multiplier * smtutil::Expression(bigint(1) << 256) + rest);
	return rest;
}
