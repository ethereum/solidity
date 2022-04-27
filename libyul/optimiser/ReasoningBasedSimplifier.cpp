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

#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/optimiser/NameCollector.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>
#include <libsmtutil/Z3Interface.h>

#include <libsolutil/CommonData.h>

#include <libsolutil/BooleanLP.h>

#include <utility>
#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::smtutil;

void ReasoningBasedSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	ReasoningBasedSimplifier simpl{_context.dialect};
	// Hack to inject the boolean lp solver.
	//simpl.m_solver = make_unique<BooleanLPSolver>();
	simpl.m_solver = make_unique<Z3Interface>();
	simpl(_ast);
}

std::optional<string> ReasoningBasedSimplifier::invalidInCurrentEnvironment()
{
	return nullopt;
}

void ReasoningBasedSimplifier::operator()(VariableDeclaration& _varDecl)
{
	SMTSolver::encodeVariableDeclaration(_varDecl);
}

void ReasoningBasedSimplifier::operator()(Assignment& _assignment)
{
	SMTSolver::encodeVariableAssignment(_assignment);
}

void ReasoningBasedSimplifier::operator()(If& _if)
{
	checkIfConditionRedundant(_if);

	decltype(m_variableSequenceCounter) oldCounters(m_variableSequenceCounter);
	optional<smtutil::Expression> oldPathCondition = m_pathCondition;

	// TODO do not call encode again - assert it is a variable
	smtutil::Expression ifCondition = encodeExpression(*_if.condition);
	smtutil::Expression branchCondition = (ifCondition >= 1);
	if (m_pathCondition)
		m_pathCondition = *m_pathCondition && branchCondition;
	else
		m_pathCondition = branchCondition;
	ASTModifier::operator()(_if.body);

	// join control flow
	for (auto& var: oldCounters)
		if (m_variableSequenceCounter.at(var.first) != var.second)
		{
			YulString const& name = var.first;
			size_t oldCounter = var.second;
			size_t branchCounter = m_variableSequenceCounter.at(name);
			size_t newCounter = branchCounter + 1;
			m_solver->newVariable(variableNameAtIndex(name, newCounter), defaultSort());

			// TODO full path condition?
			m_solver->addAssertion(
				ifCondition == 0 ||
				variableExpressionAtIndex(name, newCounter) == variableExpressionAtIndex(name, branchCounter)
			);
			m_solver->addAssertion(
				ifCondition >= 1 ||
				variableExpressionAtIndex(name, newCounter) == variableExpressionAtIndex(name, oldCounter)
			);
			var.second = newCounter;
		}

	m_variableSequenceCounter = move(oldCounters);
	m_pathCondition = move(oldPathCondition);
}

void ReasoningBasedSimplifier::operator()(ForLoop& _for)
{
	// TODO handle break / continue

	decltype(m_variableSequenceCounter) oldCounters(m_variableSequenceCounter);
	optional<smtutil::Expression> oldPathCondition = m_pathCondition;

	// TODO do not call encode again - assert it is a variable
	smtutil::Expression forCondition = encodeExpression(*_for.condition);
	smtutil::Expression branchCondition = (forCondition >= 1);
	if (m_pathCondition)
		m_pathCondition = *m_pathCondition && branchCondition;
	else
		m_pathCondition = branchCondition;
	yulAssert(_for.pre.statements.empty());

	// clear variables assigned inside body and post
	for (YulString const& varName: assignedVariableNames(_for.body) + assignedVariableNames(_for.post))
	{
		m_variableSequenceCounter[varName]++;
		m_solver->newVariable(variableNameAtIndex(varName, m_variableSequenceCounter.at(varName)), defaultSort());
		restrictToEVMWord(currentVariableExpression(varName));
	}

	ASTModifier::operator()(_for.body);
	// TODO clear modified variables! - but only if there is a 'break'
	ASTModifier::operator()(_for.post);

	// clear variables assigned inside body and post
	for (YulString const& varName: assignedVariableNames(_for.body) + assignedVariableNames(_for.post))
	{
		m_variableSequenceCounter[varName]++;
		m_solver->newVariable(variableNameAtIndex(varName, m_variableSequenceCounter.at(varName)), defaultSort());
		restrictToEVMWord(currentVariableExpression(varName));
	}

	m_variableSequenceCounter = move(oldCounters);
	m_pathCondition = move(oldPathCondition);
}

void ReasoningBasedSimplifier::operator()(FunctionCall& _fun)
{
	// TODO in general, a big problem is that all expressions involving
	// literals have to be evaluated (expression simplifier)
	// and rematerialized (literal rematerializer)
	// otherwise, the mult by constant does not work.




	ASTModifier::operator()(_fun);
	// TODO do not forget to add path condition!
	// TODO and(x, 0xfff) -> x if x <= 0xfff
	// TODO if _fun is not returning, assert that the path condition is aflse
	// -> This should be the job of the structural simplifier!
}

void ReasoningBasedSimplifier::operator()(FunctionDefinition& _fun)
{
	ScopedSaveAndRestore counters(m_variableSequenceCounter, {});
	ScopedSaveAndRestore pathCond(m_pathCondition, true);
	for (auto const& param: _fun.parameters)
		encodeVariableUpdateUnknown(param.name);
	for (auto const& retVar: _fun.returnVariables)
	{
		encodeVariableUpdateUnknown(retVar.name);
		// TODO remove the redundant encoding above
		m_solver->addAssertion(currentVariableExpression(retVar.name) == 0);
	}

	ASTModifier::operator()(_fun);
}

ReasoningBasedSimplifier::ReasoningBasedSimplifier(
	Dialect const& _dialect
):
	SMTSolver(_dialect)
{
}

void ReasoningBasedSimplifier::checkIfConditionRedundant(If& _if)
{
	if (!SideEffectsCollector{m_dialect, *_if.condition}.movable())
		return;

	cout << "Checking if condition  can be false" << endl;
	// TODO should not call encode, but instead check if it is
	// a variable and use its name / value
	smtutil::Expression condition = encodeExpression(*_if.condition);
	m_solver->push();
	// TODO find a way so that we do not have to do that all the time.
	if (m_pathCondition)
		m_solver->addAssertion(*m_pathCondition);
	m_solver->addAssertion(condition == constantValue(0));
	cout << "  running check" << endl;
	CheckResult result = m_solver->check({}).first;
	m_solver->pop();
	if (result == CheckResult::UNSATISFIABLE)
	{
		cout << " unsat => cannot be false!" << endl;
		Literal trueCondition = m_dialect.trueLiteral();
		trueCondition.debugData = debugDataOf(*_if.condition);
		_if.condition = make_unique<yul::Expression>(move(trueCondition));
	}
	else
	{
		cout << "Checking if condition  can be true" << endl;
		m_solver->push();
		// TODO find a way so that we do not have to do that all the time.
		if (m_pathCondition)
			m_solver->addAssertion(*m_pathCondition);
		m_solver->addAssertion(condition >= 1);
		cout << "  running check" << endl;
		CheckResult result2 = m_solver->check({}).first;
		m_solver->pop();
		if (result2 == CheckResult::UNSATISFIABLE)
		{
			cout << " unsat => cannot be true!" << endl;
			Literal falseCondition = m_dialect.zeroLiteralForType(m_dialect.boolType);
			falseCondition.debugData = debugDataOf(*_if.condition);
			_if.condition = make_unique<yul::Expression>(move(falseCondition));
			_if.body = yul::Block{};
		}
		cout << " unknown :(" << endl;
	}
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
	{
		auto result = arguments.at(0) + arguments.at(1) - (bigint(1) << 256) * newZeroOneVariable();
		restrictToEVMWord(result);
		return result;
	}
	case evmasm::Instruction::MUL:
		// TODO this only works will with the rematerializer.
		if (holds_alternative<Literal>(_arguments.at(0)) || holds_alternative<Literal>(_arguments.at(1)))
			return wrap(arguments.at(0) * arguments.at(1));
		else
			return newRestrictedVariable();
	case evmasm::Instruction::SUB:
	{
		auto result = arguments.at(0) - arguments.at(1) + (bigint(1) << 256) * newZeroOneVariable();
		restrictToEVMWord(result);
		return result;
	}
	case evmasm::Instruction::DIV:
		break;
		/*
		// TODO add assertion that result is <= input
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			arguments.at(0) / arguments.at(1)
		);
		*/
	case evmasm::Instruction::SDIV:
		break;
		/*
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			// No `wrap()` needed here, because -2**255 / -1 results
			// in 2**255 which is "converted" to its two's complement
			// representation 2**255 in `signedToTwosComplement`
			signedToTwosComplement(smtutil::signedDivisionEVM(
				twosComplementToUpscaledUnsigned(arguments.at(0)),
				twosComplementToUpscaledUnsigned(arguments.at(1))
			))
		);
		*/
	case evmasm::Instruction::MOD:
		break;
		/*
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			arguments.at(0) % arguments.at(1)
		);
		*/
	case evmasm::Instruction::SMOD:
		break;
		/*
		return smtutil::Expression::ite(
			arguments.at(1) == constantValue(0),
			constantValue(0),
			signedToTwosComplement(signedModuloEVM(
				twosComplementToUpscaledUnsigned(arguments.at(0)),
				twosComplementToUpscaledUnsigned(arguments.at(1))
			))
		);
		*/
	case evmasm::Instruction::LT:
		return booleanValue(arguments.at(0) < arguments.at(1));
	case evmasm::Instruction::SLT:
		return booleanValue(
			twosComplementToUpscaledUnsigned(arguments.at(0)) + smtutil::Expression(bigint(1) << 256) <
			twosComplementToUpscaledUnsigned(arguments.at(1)) + smtutil::Expression(bigint(1) << 256)
		);
	case evmasm::Instruction::GT:
		return booleanValue(arguments.at(0) > arguments.at(1));
	case evmasm::Instruction::SGT:
		return booleanValue(
			twosComplementToUpscaledUnsigned(arguments.at(0)) + smtutil::Expression(bigint(1) << 256) >
			twosComplementToUpscaledUnsigned(arguments.at(1)) + smtutil::Expression(bigint(1) << 256)
		);
	case evmasm::Instruction::EQ:
		return booleanValue(arguments.at(0) == arguments.at(1));
	case evmasm::Instruction::ISZERO:
		return booleanValue(arguments.at(0) == constantValue(0));
	case evmasm::Instruction::AND:
	{
		smtutil::Expression result = newRestrictedVariable();
		m_solver->addAssertion(result <= arguments.at(0));
		m_solver->addAssertion(result <= arguments.at(1));
		// TODO can we say more?
		return result;
	}
	case evmasm::Instruction::OR:
		return smtutil::Expression::ite(
			arguments.at(0) + arguments.at(1) <= 2,
			booleanValue(arguments.at(0) + arguments.at(1) >= 1),
			// TODO we could probably restrict it a bit more
			newRestrictedVariable()
		);
	case evmasm::Instruction::XOR:
		break;
		//return bv2int(int2bv(arguments.at(0)) ^ int2bv(arguments.at(1)));
	case evmasm::Instruction::NOT:
		return smtutil::Expression(u256(-1)) - arguments.at(0);
	case evmasm::Instruction::SHL:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			newRestrictedVariable() // TODO bv2int(int2bv(arguments.at(1)) << int2bv(arguments.at(0)))
		);
	case evmasm::Instruction::SHR:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			newRestrictedVariable() // TODO bv2int(int2bv(arguments.at(1)) >> int2bv(arguments.at(0)))
		);
	case evmasm::Instruction::SAR:
		return smtutil::Expression::ite(
			arguments.at(0) > 255,
			constantValue(0),
			newRestrictedVariable() // TODO bv2int(smtutil::Expression::ashr(int2bv(arguments.at(1)), int2bv(arguments.at(0))))
		);
	case evmasm::Instruction::ADDMOD:
		break;
		/*
		return smtutil::Expression::ite(
			arguments.at(2) == constantValue(0),
			constantValue(0),
			(arguments.at(0) + arguments.at(1)) % arguments.at(2)
		);
		*/
	case evmasm::Instruction::MULMOD:
		break;
		/*
		return smtutil::Expression::ite(
			arguments.at(2) == constantValue(0),
			constantValue(0),
			(arguments.at(0) * arguments.at(1)) % arguments.at(2)
		);
		*/
	// TODO SIGNEXTEND
	default:
		break;
	}
	return newRestrictedVariable();
}

smtutil::Expression ReasoningBasedSimplifier::newZeroOneVariable()
{
	smtutil::Expression var = newVariable();
	m_solver->addAssertion(var <= 1);
	m_solver->addAssertion(var <= 0 || var >= 1);
	return var;
}

void ReasoningBasedSimplifier::restrictToEVMWord(smtutil::Expression _value)
{
	m_solver->addAssertion(0 <= _value && _value < bigint(1) << 256);
}
