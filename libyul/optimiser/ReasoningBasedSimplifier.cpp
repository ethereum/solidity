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
#include <libyul/Utilities.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/Helpers.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/BooleanLP.h>
#include <libsolutil/Visitor.h>

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

	// TODO for a boolean variable, we could check if it is
	// a constant.

	smtutil::Expression variable = newRestrictedVariable(
		"yul_" + varName.str(),
		_varDecl.value && isBoolean(*_varDecl.value)
	);
	bool const inserted = m_variables.insert({varName, variable}).second;
	yulAssert(inserted, "");
	if (!_varDecl.value)
		return; // TODO we could encode zero, but the variable should not be used anyway.

	std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (auto const* builtin = dialect->builtin(_functionCall.functionName.name))
					if (builtin->instruction)
							handleDeclaration(varName, *builtin->instruction, _functionCall.arguments);
		},
		[&](Identifier const& _identifier)
		{
			if (
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				m_solver->addAssertion(variable == m_variables.at(_identifier.name));
		},
		[&](Literal const& _literal)
		{
			// TODO could avoid encoding the restrictions.
			m_solver->addAssertion(variable == literalValue(_literal));
		}
	}, *_varDecl.value);

	if (_varDecl.value && isBoolean(*_varDecl.value) && SideEffectsCollector{m_dialect, *_varDecl.value}.movable())
	{
		// TODO if we use this, then it actually already handles the if statement case...
		if (makesInfeasible(!variable))
			// TODO debug data
			_varDecl.value = make_unique<yul::Expression>(m_dialect.trueLiteral());
		else if (makesInfeasible(variable))
			_varDecl.value = make_unique<yul::Expression>(m_dialect.zeroLiteralForType({}));
	}
// TODO could use 	SMTSolver::encodeVariableDeclaration(_varDecl);
}

void ReasoningBasedSimplifier::operator()(If& _if)
{
	if (!holds_alternative<Identifier>(*_if.condition))
	{
		ASTModifier::operator()(_if.body);
		return;
	}
	Identifier const& condition = get<Identifier>(*_if.condition);
	if (!m_ssaVariables.count(condition.name) || !m_variables.count(condition.name))
	{
		ASTModifier::operator()(_if.body);
		return;
	}
	smtutil::Expression cond = m_variables.at(condition.name);

	bool constantTrue = makesInfeasible(
		isBoolean(*_if.condition) ?
		!cond :
		(cond == bigint(0))
	);

	if (constantTrue)
	{
		Literal trueCondition = m_dialect.trueLiteral();
		trueCondition.debugData = debugDataOf(*_if.condition);
		_if.condition = make_unique<yul::Expression>(move(trueCondition));
	}
	else
	{
		bool constantFalse = makesInfeasible(
			isBoolean(*_if.condition) ?
			cond :
			(cond >= bigint(1))
		);

		if (constantFalse)
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
	if (isBoolean(*_if.condition))
		m_solver->addAssertion(cond);
	else
		m_solver->addAssertion(cond >= bigint(1));

	ASTModifier::operator()(_if.body);
	m_solver->pop();

	// TODO if we do this, we have to push/pop inside for loops because
	// 'leave' / 'continue' is considered terminating.
	bool bodyTerminates =
		!_if.body.statements.empty() &&
		TerminationFinder(m_dialect).controlFlowKind(_if.body.statements.back()) !=
		TerminationFinder::ControlFlow::FlowOut;

	if (bodyTerminates)
	{
		//cout << "Body always terminates." << endl;
		if (isBoolean(*_if.condition))
			m_solver->addAssertion(!cond);
		else
			m_solver->addAssertion(cond == bigint(0));
	}
}

// TODO switch also needs push/pop?

// TODO does break/continue need pecial handling in loops?
// TODO leave in functions?

void ReasoningBasedSimplifier::operator()(ForLoop& _for)
{
	(*this)(_for.pre);
	visit(*_for.condition);
	m_solver->push();

	Identifier const* condition = get_if<Identifier>(_for.condition.get());
	if (condition && m_ssaVariables.count(condition->name) && m_variables.count(condition->name))
	{
		smtutil::Expression cond = m_variables.at(condition->name);
		if (isBoolean(*_for.condition))
			m_solver->addAssertion(cond);
		else
			m_solver->addAssertion(cond >= bigint(1));
	}
	(*this)(_for.body);
	(*this)(_for.post);

	m_solver->pop();
}

void ReasoningBasedSimplifier::operator()(FunctionDefinition& _fun)
{
	m_solver->push();
	ASTModifier::operator()(_fun);
	m_solver->pop();
}

ReasoningBasedSimplifier::ReasoningBasedSimplifier(
	Dialect const& _dialect,
	set<YulString> const& _ssaVariables
):
	m_dialect(_dialect),
	m_ssaVariables(_ssaVariables),
	m_solver(make_unique<util::BooleanLPSolver>())
{
}

void ReasoningBasedSimplifier::handleDeclaration(
	YulString _varName,
	evmasm::Instruction _instruction,
	vector<yul::Expression> const& _arguments
)
{
	smtutil::Expression variable = m_variables.at(_varName);
	vector<smtutil::Expression> arguments;
	for (yul::Expression const& arg: _arguments)
	{
		if (holds_alternative<Identifier>(arg))
		{
			Identifier const& v = get<Identifier>(arg);
			if (!m_ssaVariables.count(v.name) || !m_variables.count(v.name))
				return;
			arguments.push_back(m_variables.at(v.name));
		}
		else if (holds_alternative<Literal>(arg))
			arguments.push_back(literalValue(get<Literal>(arg)));
		else
			return;
	}

	optional<smtutil::Expression> x;
	optional<smtutil::Expression> y;
	optional<smtutil::Expression> z;
	if (arguments.size() > 0)
		x = arguments.at(0);
	if (arguments.size() > 1)
		y = arguments.at(1);
	if (arguments.size() > 2)
		z = arguments.at(2);

	switch (_instruction)
	{
	case evmasm::Instruction::ADD:
	{
		smtutil::Expression overflow = m_solver->newVariable(uniqueName(), SortProvider::boolSort);
		m_solver->addAssertion(overflow || (variable == *x + *y));
		m_solver->addAssertion(!overflow || (variable == *x + *y - smtutil::Expression(bigint(1) << 256)));
		break;
	}
	case evmasm::Instruction::SUB:
	{
		smtutil::Expression underflow = m_solver->newVariable(uniqueName(), SortProvider::boolSort);
		m_solver->addAssertion(underflow || (variable == *x - *y));
		m_solver->addAssertion(!underflow || (variable == *x - *y + smtutil::Expression(bigint(1) << 256)));
		break;
	}
	//case evmasm::Instruction::MUL:
		// TODO encode constants?
	case evmasm::Instruction::DIV:
		m_solver->addAssertion(variable <= *x);
		break;
	case evmasm::Instruction::ADDMOD:
		m_solver->addAssertion(variable < *z);
		break;
	case evmasm::Instruction::SHL:
		if (holds_alternative<Literal>(_arguments.at(0)))
		{
			u256 shiftAmount = valueOfLiteral(get<Literal>(_arguments.at(0)));
			cout << "shift by " << shiftAmount << endl;
		}
		break;
	case evmasm::Instruction::SHR:
		if (holds_alternative<Literal>(_arguments.at(0)))
		{
			u256 shiftAmount = valueOfLiteral(get<Literal>(_arguments.at(0)));
			cout << "shift by " << shiftAmount << endl;
		}
		break;
	case evmasm::Instruction::SAR:
		break;
	case evmasm::Instruction::LT:
		m_solver->addAssertion(variable == (*x < *y));
		break;
	case evmasm::Instruction::GT:
		m_solver->addAssertion(variable == (*x > *y));
		break;
	//	case evmasm::Instruction::SLT:
	//	case evmasm::Instruction::SGT:
	// TODO
	case evmasm::Instruction::EQ:
		if (isBoolean(_arguments.at(0)) == isBoolean(_arguments.at(1)))
			m_solver->addAssertion(variable == (*x == *y));
		else if (isBoolean(_arguments.at(0)))
			m_solver->addAssertion(variable == ((*x && *y >= 1) || (!*x && *y == 0)));
		else
			m_solver->addAssertion(variable == ((*y && *x >= 1) || (!*y && *x == 0)));
		break;
	case evmasm::Instruction::ISZERO:
		if (isBoolean(_arguments.at(0)))
			m_solver->addAssertion(variable == (!*x));
		else
			m_solver->addAssertion(variable == (*x <= smtutil::Expression(bigint(0))));
		break;
	case evmasm::Instruction::NOT:
		if (isBoolean(_arguments.at(0)))
			m_solver->addAssertion(variable == (!*x));
		else
			// TODO is this correct?
			m_solver->addAssertion(variable == ((smtutil::Expression(bigint(1) << 256) - 1) - *x));
		break;
	case evmasm::Instruction::AND:
		if (m_booleanVariables.count(_varName.str()))
			m_solver->addAssertion(variable == (*x && *y));
		else
			m_solver->addAssertion(variable <= *x && variable <= *y);
		break;
	case evmasm::Instruction::OR:
		if (m_booleanVariables.count(_varName.str()))
			m_solver->addAssertion(variable == (*x || *y));
		else
		{
			m_solver->addAssertion(variable >= *x && variable >= *y);
			m_solver->addAssertion(variable <= *x + *y);
		}
		break;
	// TODO all builtins whose return values can be restricted.
	default:
		cout << "Not handling instruction " << evmasm::instructionInfo(_instruction).name << endl;
		break;
	}
}

smtutil::Expression ReasoningBasedSimplifier::newRestrictedVariable(string const& _name, bool _boolean)
{
	string name = _name.empty() ? uniqueName() : _name;
	if (_boolean)
		m_booleanVariables.insert(name);
	smtutil::Expression var = m_solver->newVariable(name, _boolean ? SortProvider::boolSort : defaultSort());
	if (!_boolean)
		m_solver->addAssertion(var <= smtutil::Expression(bigint(1) << 256) - 1);
	return var;
}

string ReasoningBasedSimplifier::uniqueName()
{
	return "expr_" + to_string(m_varCounter++);
}
bool ReasoningBasedSimplifier::makesInfeasible(smtutil::Expression _constraint)
{
	m_solver->push();
	m_solver->addAssertion(_constraint);
	bool result = infeasible();
	m_solver->pop();
	return result;
}

bool ReasoningBasedSimplifier::feasible()
{
	CheckResult result = m_solver->check({}).first;
	return result == CheckResult::SATISFIABLE;
}

bool ReasoningBasedSimplifier::infeasible()
{
	CheckResult result = m_solver->check({}).first;
	return result == CheckResult::UNSATISFIABLE;
}

YulString ReasoningBasedSimplifier::localVariableFromExpression(string const& _expressionName)
{
	solAssert(_expressionName.substr(0, 4) == "yul_", "");
	return YulString(_expressionName.substr(4));

}

bool ReasoningBasedSimplifier::isBoolean(Expression const& _expression) const
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& _functionCall)
		{
			if (auto const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
				if (auto const* builtin = dialect->builtin(_functionCall.functionName.name))
							  // TODO assert
					switch (*builtin->instruction)
					{
					case evmasm::Instruction::LT:
					case evmasm::Instruction::GT:
					case evmasm::Instruction::SLT:
					case evmasm::Instruction::SGT:
					case evmasm::Instruction::EQ:
					case evmasm::Instruction::ISZERO:
						return true;
					case evmasm::Instruction::AND:
					case evmasm::Instruction::OR:
						return
							isBoolean(_functionCall.arguments.at(0)) &&
							isBoolean(_functionCall.arguments.at(1));
					case evmasm::Instruction::NOT:
						return isBoolean(_functionCall.arguments.at(0));
					default:
						break;
					}
			return false;
		},
		[&](Identifier const& _identifier) -> bool
		{
			return m_booleanVariables.count("yul_" + _identifier.name.str());
		},
		[&](Literal const& _literal)
		{
			return _literal.kind == LiteralKind::Boolean;
		}
	}, _expression);
}

shared_ptr<Sort> ReasoningBasedSimplifier::defaultSort() const
{
	return SortProvider::intSort();
}

smtutil::Expression ReasoningBasedSimplifier::literalValue(Literal const& _literal) const
{
	return smtutil::Expression(valueOfLiteral(_literal));
}
