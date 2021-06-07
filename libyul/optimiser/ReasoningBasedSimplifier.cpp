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
#include <libyul/optimiser/SMTSolver.h>

#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

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
	SMTSolver::encodeVariableDeclaration(_varDecl);
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
		trueCondition.debugData = debugDataOf(*_if.condition);
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
			falseCondition.debugData = debugDataOf(*_if.condition);
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
	SMTSolver(_ssaVariables, _dialect),
	m_dialect(_dialect)
{
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
			// representation 2**255 in `signedToTwosComplement`
			signedToTwosComplement(smtutil::signedDivisionEVM(
				twosComplementToSigned(arguments.at(0)),
				twosComplementToSigned(arguments.at(1))
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
			signedToTwosComplement(signedModuloEVM(
				twosComplementToSigned(arguments.at(0)),
				twosComplementToSigned(arguments.at(1))
			))
		);
	case evmasm::Instruction::LT:
		return booleanValue(arguments.at(0) < arguments.at(1));
	case evmasm::Instruction::SLT:
		return booleanValue(twosComplementToSigned(arguments.at(0)) < twosComplementToSigned(arguments.at(1)));
	case evmasm::Instruction::GT:
		return booleanValue(arguments.at(0) > arguments.at(1));
	case evmasm::Instruction::SGT:
		return booleanValue(twosComplementToSigned(arguments.at(0)) > twosComplementToSigned(arguments.at(1)));
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
