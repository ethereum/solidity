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

/// Property test for the shape of the code that Yul optimizer steps leave behind
///
/// The test generates a program, runs a random sequence of optimizer steps on it, followed by one of the steps
/// below, and checks the shape that step promises:
///   - FunctionHoister (h): all function definitions are in the outermost block
///   - FunctionGrouper (g): the outermost block is a block followed by function definitions only
///   - ForLoopInitRewriter (o): no loop has an initializer
///   - ExpressionSplitter (x): outside of loop conditions, the arguments of calls, the conditions of ifs and the
///     expressions of switches are variables or literals
///   - SSATransform (a): every assignment assigns one variable the value of a variable that is itself never
///     assigned, and a variable that is assigned is only read as the whole value of a declaration or in a loop
///     condition, unless it is the return variable of a function
/// Whatever the sequence, the result also has to pass the analyzer and declare every name only once.
///
/// The programs come from YulProgramGenerator.h, which also lists what they do not cover.
///
/// Limitations:
///   - nothing is executed: a step that keeps the shape but changes the behaviour passes

#include <test/fuzztest/libyul/OptimizerStepSequence.h>
#include <test/fuzztest/libyul/YulProgramGenerator.h>

#include <libyul/AST.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Dialect.h>
#include <libyul/Object.h>
#include <libyul/YulStack.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameCollector.h>

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

#include <boost/exception/diagnostic_information.hpp>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <fmt/format.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

namespace
{

std::size_t constexpr maxRandomSteps = 12;

struct Statistics: testing::Environment
{
	GeneratorStatistics generator;

	void TearDown() override { generator.print(); }
};

Statistics& statistics = *static_cast<Statistics*>(testing::AddGlobalTestEnvironment(new Statistics));

/// The steps whose shape is checked, by their abbreviations
enum class CheckedStep: char
{
	FunctionHoister = 'h',
	FunctionGrouper = 'g',
	ForLoopInitRewriter = 'o',
	ExpressionSplitter = 'x',
	SSATransform = 'a'
};

std::vector<CheckedStep> const checkedSteps{
	CheckedStep::FunctionHoister,
	CheckedStep::FunctionGrouper,
	CheckedStep::ForLoopInitRewriter,
	CheckedStep::ExpressionSplitter,
	CheckedStep::SSATransform
};

char format_as(CheckedStep const _step) { return static_cast<char>(_step); }

class ShapeChecker: public ASTWalker
{
public:
	std::optional<std::string> violation;

protected:
	void report(std::string _violation)
	{
		if (!violation)
			violation = std::move(_violation);
	}
};

class DeclaredOnce: public ShapeChecker
{
public:
	using ASTWalker::operator();
	void operator()(VariableDeclaration const& _declaration) override
	{
		for (NameWithDebugData const& variable: _declaration.variables)
			declare(variable.name);
		ASTWalker::operator()(_declaration);
	}
	void operator()(FunctionDefinition const& _function) override
	{
		declare(_function.name);
		for (NameWithDebugData const& variable: _function.parameters)
			declare(variable.name);
		for (NameWithDebugData const& variable: _function.returnVariables)
			declare(variable.name);
		ASTWalker::operator()(_function);
	}

private:
	void declare(YulName const& _name)
	{
		if (!m_declared.insert(_name).second)
			report(fmt::format("{} is declared more than once", _name.str()));
	}

	std::set<YulName> m_declared;
};

class FunctionsInOutermostBlock: public ShapeChecker
{
public:
	using ASTWalker::operator();
	void operator()(Block const& _block) override
	{
		++m_blockDepth;
		ASTWalker::operator()(_block);
		--m_blockDepth;
	}
	void operator()(FunctionDefinition const& _function) override
	{
		if (m_blockDepth != 1)
			report(fmt::format("function {} is not in the outermost block", _function.name.str()));
		ASTWalker::operator()(_function);
	}

private:
	std::size_t m_blockDepth = 0;
};

class NoLoopInitializers: public ShapeChecker
{
public:
	using ASTWalker::operator();
	void operator()(ForLoop const& _loop) override
	{
		if (!_loop.pre.statements.empty())
			report("a loop has an initializer");
		ASTWalker::operator()(_loop);
	}
};

class ExpressionsSplit: public ShapeChecker
{
public:
	explicit ExpressionsSplit(Dialect const& _dialect): m_dialect(_dialect) {}

	using ASTWalker::operator();
	void operator()(FunctionCall const& _call) override
	{
		for (Expression const& argument: _call.arguments)
			requireSimple(argument, "an argument of a call");
	}
	void operator()(If const& _if) override
	{
		requireSimple(*_if.condition, "the condition of an if");
		(*this)(_if.body);
	}
	void operator()(Switch const& _switch) override
	{
		requireSimple(*_switch.expression, "the expression of a switch");
		for (Case const& switchCase: _switch.cases)
			(*this)(switchCase.body);
	}
	void operator()(ForLoop const& _loop) override
	{
		// The step leaves the condition alone: there is no place to move its parts to.
		(*this)(_loop.pre);
		(*this)(_loop.body);
		(*this)(_loop.post);
	}

private:
	void requireSimple(Expression const& _expression, char const* _role)
	{
		if (std::holds_alternative<FunctionCall>(_expression))
			report(fmt::format("{} is a call: {}", _role, std::visit(AsmPrinter{m_dialect}, _expression)));
	}

	Dialect const& m_dialect;
};

class SingleAssignment: public ShapeChecker
{
public:
	SingleAssignment(Dialect const& _dialect, Block const& _ast):
		m_dialect(_dialect), m_assigned(assignedVariableNames(_ast))
	{}

	using ASTWalker::operator();
	void operator()(Assignment const& _assignment) override
	{
		Identifier const* value = std::get_if<Identifier>(_assignment.value.get());
		if (_assignment.variableNames.size() != 1 || !value || m_assigned.contains(value->name))
			report(fmt::format(
				"an assignment does not assign one variable the value of a variable that is never assigned: {}",
				AsmPrinter{m_dialect}(_assignment)
			));
	}
	void operator()(VariableDeclaration const& _declaration) override
	{
		if (!_declaration.value)
			return;
		// `let a_1 := a` where control flow joins and `let a := a_1` are how an assigned variable gets its value
		// read and set.
		if (std::holds_alternative<Identifier>(*_declaration.value))
			return;
		for (NameWithDebugData const& variable: _declaration.variables)
			if (m_assigned.contains(variable.name))
				report(fmt::format(
					"a variable that is assigned is not declared with the value of a variable: {}",
					AsmPrinter{m_dialect}(_declaration)
				));
		visit(*_declaration.value);
	}
	void operator()(ForLoop const& _loop) override
	{
		// The step leaves the condition alone: there is no place to declare a variable for it.
		(*this)(_loop.pre);
		(*this)(_loop.body);
		(*this)(_loop.post);
	}
	void operator()(FunctionDefinition const& _function) override
	{
		// The step only gives parameters a variable to be read through from the start of the function on.
		for (NameWithDebugData const& variable: _function.returnVariables)
			m_returnVariables.insert(variable.name);
		ASTWalker::operator()(_function);
	}
	void operator()(Identifier const& _identifier) override
	{
		if (m_assigned.contains(_identifier.name) && !m_returnVariables.contains(_identifier.name))
			report(fmt::format("{} is assigned, and read inside an expression", _identifier.name.str()));
	}

private:
	Dialect const& m_dialect;
	std::set<YulName> m_assigned;
	std::set<YulName> m_returnVariables;
};

/// @returns how @a _ast breaks the shape that @a _step promises, if it does.
std::optional<std::string> brokenShape(CheckedStep const _step, Dialect const& _dialect, Block const& _ast)
{
	auto const check = [&](ShapeChecker&& _checker) {
		_checker(_ast);
		return _checker.violation;
	};
	switch (_step)
	{
	case CheckedStep::FunctionHoister:
		return check(FunctionsInOutermostBlock{});
	case CheckedStep::FunctionGrouper:
	{
		bool grouped = !_ast.statements.empty() && std::holds_alternative<Block>(_ast.statements.front());
		for (std::size_t i = 1; i < _ast.statements.size(); ++i)
			grouped = grouped && std::holds_alternative<FunctionDefinition>(_ast.statements[i]);
		if (grouped)
			return std::nullopt;
		return "the outermost block is not a block followed by function definitions only";
	}
	case CheckedStep::ForLoopInitRewriter:
		return check(NoLoopInitializers{});
	case CheckedStep::ExpressionSplitter:
		return check(ExpressionsSplit{_dialect});
	case CheckedStep::SSATransform:
		return check(SingleAssignment{_dialect, _ast});
	}
	util::unreachable();
}

/// Runs @a _steps and then @a _checkedStep on @a _source, with the steps these need before them, and asserts
/// that the result has the shape that @a _checkedStep promises, passes the analyzer and declares every name once.
void checkShape(std::string const& _source, std::string const& _steps, CheckedStep const _checkedStep)
{
	YulStack stack;
	ASSERT_TRUE(stack.parseAndAnalyze("source", _source)) << _source;
	std::shared_ptr<Object const> const object = stack.parserResult();
	Dialect const& dialect = *object->dialect();

	std::string const steps = _steps + format_as(_checkedStep);
	std::string const sequence = prerequisitesOf(steps) + steps;
	Block const ast = runSteps(*object, sequence);
	auto const context = [&] {
		return fmt::format("steps: {}\nresult:\n{}\nsource:\n{}", sequence, AsmPrinter{dialect}(ast), _source);
	};

	try
	{
		AsmAnalyzer::analyzeStrictAssertCorrect(dialect, ast, object->summarizeStructure());
	}
	catch (util::Exception const& _exception)
	{
		FAIL() << "The result does not pass the analyzer: " << _exception.what() << "\n" << context();
	}
	DeclaredOnce declaredOnce;
	declaredOnce(ast);
	ASSERT_FALSE(declaredOnce.violation.has_value()) << *declaredOnce.violation << "\n" << context();

	std::optional<std::string> const broken = brokenShape(_checkedStep, dialect, ast);
	ASSERT_FALSE(broken.has_value()) << fmt::format(
		"The result does not have the shape that step {} promises: {}\n{}",
		_checkedStep,
		*broken,
		context()
	);
}

struct FuzzInput
{
	std::vector<Instruction> instructions;
	std::string randomSteps;
	CheckedStep checkedStep;
};

auto inputDomain()
{
	return fuzztest::StructOf<FuzzInput>(
		instructionsDomain(),
		stepsDomain(maxRandomSteps),
		fuzztest::ElementOf(checkedSteps)
	);
}

}

static void StepsLeavePromisedShape(FuzzInput const& _input)
{
	std::string const source = buildProgram(_input.instructions, ProgramProfile::Aliasing, &statistics.generator);
	try
	{
		checkShape(source, _input.randomSteps, _input.checkedStep);
	}
	catch (...)
	{
		FAIL() << fmt::format(
			"Exception: {}\nrandom steps: {}\nchecked step: {}\nsource:\n{}",
			boost::current_exception_diagnostic_information(),
			_input.randomSteps,
			_input.checkedStep,
			source
		);
	}
}

FUZZ_TEST(OptimizerStepProperty, StepsLeavePromisedShape)
	.WithDomains(inputDomain());

/// Code that has none of the shapes, so that every checked step has to change it.
TEST(OptimizerStepProperty, StepsChangeCodeOfOtherShape)
{
	std::string const source = R"({
		let a := calldataload(0)
		for { let i := add(a, mload(a)) } lt(i, add(a, 3)) { i := add(i, mload(i)) } {
			function inner(p) -> r {
				r := add(p, sload(mul(p, 2)))
				if lt(sload(r), mload(p)) { r := 1 }
			}
			a := inner(add(a, sload(i)))
		}
		switch mload(add(a, 1))
		case 0 { a := 2 }
		default { a := mul(a, 3) }
		sstore(a, add(a, mload(a)))
	})";
	YulStack stack;
	ASSERT_TRUE(stack.parseAndAnalyze("source", source));
	std::shared_ptr<Object const> const object = stack.parserResult();
	Block const unchanged = runSteps(*object, "");
	for (CheckedStep const step: checkedSteps)
	{
		EXPECT_TRUE(brokenShape(step, *object->dialect(), unchanged).has_value()) << format_as(step);
		EXPECT_NO_FATAL_FAILURE(checkShape(source, "", step)) << format_as(step);
	}
}

}
