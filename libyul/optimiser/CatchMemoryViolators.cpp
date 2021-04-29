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

#include <libyul/optimiser/CatchMemoryViolators.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/optimiser/SSAValueTracker.h>

#include <libevmasm/SemanticInformation.h>
#include <libyul/AsmPrinter.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Visitor.h>

#include <libsmtutil/SolverInterface.h>
#include <optional>

namespace
{
	// Copied from libsolidity/codegen/CompilerUtils.h, to fix linking issues for tools.
	unsigned constexpr freeMemoryPointer = 256;
	unsigned constexpr generalPurposeMemoryStart = 256 + 64;
}

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::smtutil;
using namespace solidity::evmasm;

void CatchMemoryViolators::operator()(VariableDeclaration const& _variableDeclaration)
{
	ASTWalker::operator()(_variableDeclaration);
	Solver::encodeVariableDeclaration(_variableDeclaration);
}

smtutil::Expression CatchMemoryViolators::safeMemoryRestrictedVariable()
{
	smtutil::Expression var = newVariable();
	m_solver->addAssertion(generalPurposeMemoryStart <= var && var < smtutil::Expression((bigint(1) << 32 ) - 1));
	return var;
}

smtutil::Expression CatchMemoryViolators::encodeEVMBuiltin(
	evmasm::Instruction /* _instruction */,
	std::vector<Expression> const& /* _arguments */
)
{
	yulAssert(false, "Should not reach here");
}

smtutil::Expression CatchMemoryViolators::encodeExpression(Expression const& _expression)
{
	return std::visit(GenericVisitor{
		[&](FunctionCall const& /* _functionCall */)
		{
			// A hack for dealing with mload(free_mem_ptr) and similar builtlin calls. Replacing it
			// by a "free variable" would lead to false positives.
			//
			// Returns a free variable, with a lower bound, CompilerUtils::generalPurposeMemoryStart
			// Note that CompilerUtils::generalPurposeMemoryStart was modified to be a larger than
			// usual number (now: 256 + 64 instead of 100)
			return safeMemoryRestrictedVariable();
		},
		[&](Identifier const& _identifier)
		{
			if (
				m_ssaVariables.count(_identifier.name) &&
				m_variables.count(_identifier.name)
			)
				return m_variables.at(_identifier.name);
			else
				// Again the same hack as before
				return safeMemoryRestrictedVariable();
		},
		[&](Literal const& _literal)
		{
			return literalValue(_literal);
		}
	}, _expression);
}

void CatchMemoryViolators::operator()(FunctionCall const& _functionCall)
{
	ASTWalker::operator()(_functionCall);

	optional<Instruction> instruction = toEVMInstruction(m_dialect, _functionCall.functionName.name);
	if (!instruction)
		return;

	vector<smtutil::Expression> arguments = applyMap(
		_functionCall.arguments,
		[this](yul::Expression const& _expr) { return encodeExpression(_expr); }
	);

	m_solver->push();

	// A dummy variable.
	smtutil::Expression memoryLocation = newVariable();
	m_solver->addAssertion(
		(64 <= memoryLocation) &&
		(memoryLocation < freeMemoryPointer)
	);

	// Encode a write to location [p, p + n)
	auto encodeWrite = [&](auto const& p, auto const& n)
	{
		m_solver->addAssertion(p <= memoryLocation);
		m_solver->addAssertion(memoryLocation < p + n);
		yulAssert(
			m_solver->check({}).first != CheckResult::SATISFIABLE,
			"Memory Violator Found! At " + AsmPrinter{m_dialect}(_functionCall)
		);
		m_solver->pop();
	};

	if (SemanticInformation::memory(*instruction) == SemanticInformation::Write)
		switch (*instruction)
		{
		case Instruction::CALLDATACOPY:
		case Instruction::CODECOPY:
		case Instruction::RETURNDATACOPY:
			yulAssert(arguments.size() == 3, "");
			encodeWrite(arguments.at(0), arguments.at(2));
			break;
		case Instruction::EXTCODECOPY:
			yulAssert(arguments.size() == 4, "");
			encodeWrite(arguments.at(1), arguments.at(3));
			break;
		case Instruction::MSTORE:
			yulAssert(arguments.size() == 2, "");
			encodeWrite(arguments.at(0), constantValue(32));
			break;
		case Instruction::MSTORE8:
			yulAssert(arguments.size() == 2, "");
			encodeWrite(arguments.at(0), constantValue(8));
			break;
		case Instruction::CALL:
		case Instruction::CALLCODE:
			yulAssert(arguments.size() == 7, "");
			encodeWrite(arguments.at(5), arguments.at(6));
			break;
		case Instruction::DELEGATECALL:
		case Instruction::STATICCALL:
			yulAssert(arguments.size() == 6, "");
			encodeWrite(arguments.at(4), arguments.at(5));
			break;
		default:
			yulAssert(false, "Did not encode a write operation");
		}
}

void CatchMemoryViolators::run(OptimiserStepContext& _context, Block& _ast)
{
	if (!dynamic_cast<EVMDialect const*>(&_context.dialect))
		return;

	set<YulString> ssaVariables = SSAValueTracker::ssaVariables(_ast);

	CatchMemoryViolators catcher{ssaVariables, _context.dialect};
	// Will throw if it catches.
	catcher(_ast);
}
