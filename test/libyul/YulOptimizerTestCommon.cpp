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

#include <test/libyul/YulOptimizerTestCommon.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/CircularReferencesPruner.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/EqualStoreEliminator.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/FunctionSpecializer.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/LoopInvariantCodeMotion.h>
#include <libyul/optimiser/StackLimitEvader.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/UnusedAssignEliminator.h>
#include <libyul/optimiser/UnusedStoreEliminator.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/backends/evm/ConstantOptimiser.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/CompilabilityChecker.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <random>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;

YulOptimizerTestCommon::YulOptimizerTestCommon(
	std::shared_ptr<Object const> _obj,
	Dialect const& _dialect
): m_dialect(&_dialect), m_object(_obj), m_optimizedObject(std::make_shared<Object>(*_obj))
{
	if (
		std::any_of(
			m_object->subObjects.begin(),
			m_object->subObjects.end(),
			[](auto const& subObject) { return dynamic_cast<Data*>(subObject.get()) == nullptr;}
		)
	)
		solUnimplementedAssert(false, "The current implementation of YulOptimizerTests ignores subobjects that are not Data.");

	m_namedSteps = {
		{"disambiguator", [&]() { return disambiguate(); }},
		{"nameDisplacer", [&]() {
			auto block = disambiguate();
			updateContext(block);
			NameDisplacer{
				*m_nameDispenser,
				{"illegal1"_yulname, "illegal2"_yulname, "illegal3"_yulname, "illegal4"_yulname, "illegal5"_yulname}
			}(block);
			return block;
		}},
		{"blockFlattener", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionGrouper::run(*m_context, block);
			BlockFlattener::run(*m_context, block);
			return block;
		}},
		{"constantOptimiser", [&]() {
			auto block = std::get<Block>(ASTCopier{}(m_object->code()->root()));
			updateContext(block);
			GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
			ConstantOptimiser{dynamic_cast<EVMDialect const&>(*m_dialect), meter}(block);
			return block;
		}},
		{"varDeclInitializer", [&]() {
			auto block = std::get<Block>(ASTCopier{}(m_object->code()->root()));
			updateContext(block);
			VarDeclInitializer::run(*m_context, block);
			return block;
		}},
		{"varNameCleaner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			VarNameCleaner::run(*m_context, block);
			return block;
		}},
		{"forLoopConditionIntoBody", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopConditionIntoBody::run(*m_context, block);
			return block;
		}},
		{"forLoopInitRewriter", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			return block;
		}},
		{"commonSubexpressionEliminator", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			return block;
		}},
		{"conditionalUnsimplifier", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ConditionalUnsimplifier::run(*m_context, block);
			return block;
		}},
		{"conditionalSimplifier", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ConditionalSimplifier::run(*m_context, block);
			return block;
		}},
		{"expressionSplitter", [&]() {
			auto block = std::get<Block>(ASTCopier{}(m_object->code()->root()));
			updateContext(block);
			ExpressionSplitter::run(*m_context, block);
			return block;
		}},
		{"expressionJoiner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"splitJoin", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ExpressionSplitter::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"functionGrouper", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionGrouper::run(*m_context, block);
			return block;
		}},
		{"functionHoister", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			return block;
		}},
		{"functionSpecializer", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			FunctionSpecializer::run(*m_context, block);
			return block;
		}},
		{"expressionInliner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ExpressionInliner::run(*m_context, block);
			return block;
		}},
		{"fullInliner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			FullInliner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"fullInlinerWithoutSplitter", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			FullInliner::run(*m_context, block);
			return block;
		}},
		{"rematerialiser", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			Rematerialiser::run(*m_context, block);
			return block;
		}},
		{"expressionSimplifier", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			ExpressionSimplifier::run(*m_context, block);
			ExpressionSimplifier::run(*m_context, block);
			ExpressionSimplifier::run(*m_context, block);
			UnusedPruner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"fullSimplify", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionGrouper::run(*m_context, block);
			BlockFlattener::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			ExpressionSimplifier::run(*m_context, block);
			UnusedPruner::run(*m_context, block);
			CircularReferencesPruner::run(*m_context, block);
			DeadCodeEliminator::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"unusedFunctionParameterPruner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LiteralRematerialiser::run(*m_context, block);
			UnusedFunctionParameterPruner::run(*m_context, block);
			return block;
		}},
		{"unusedPruner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			UnusedPruner::run(*m_context, block);
			return block;
		}},
		{"circularReferencesPruner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			CircularReferencesPruner::run(*m_context, block);
			return block;
		}},
		{"deadCodeEliminator", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			DeadCodeEliminator::run(*m_context, block);
			return block;
		}},
		{"ssaTransform", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			return block;
		}},
		{"unusedAssignEliminator", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			UnusedAssignEliminator::run(*m_context, block);
			return block;
		}},
		{"unusedStoreEliminator", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			UnusedStoreEliminator::run(*m_context, block);
			SSAReverser::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"equalStoreEliminator", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionHoister::run(*m_context, block);
			ForLoopInitRewriter::run(*m_context, block);
			EqualStoreEliminator::run(*m_context, block);
			return block;
		}},
		{"ssaPlusCleanup", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			UnusedAssignEliminator::run(*m_context, block);
			return block;
		}},
		{"loadResolver", [&]() {
			auto block = disambiguate();
			updateContext(block);
			FunctionGrouper::run(*m_context, block);
			BlockFlattener::run(*m_context, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			ExpressionSimplifier::run(*m_context, block);

			LoadResolver::run(*m_context, block);

			UnusedPruner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"loopInvariantCodeMotion", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LoopInvariantCodeMotion::run(*m_context, block);
			return block;
		}},
		{"controlFlowSimplifier", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			ControlFlowSimplifier::run(*m_context, block);
			return block;
		}},
		{"structuralSimplifier", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LiteralRematerialiser::run(*m_context, block);
			StructuralSimplifier::run(*m_context, block);
			return block;
		}},
		{"equivalentFunctionCombiner", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			EquivalentFunctionCombiner::run(*m_context, block);
			return block;
		}},
		{"ssaReverser", [&]() {
			auto block = disambiguate();
			updateContext(block);
			SSAReverser::run(*m_context, block);
			return block;
		}},
		{"ssaAndBack", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			// apply SSA
			SSATransform::run(*m_context, block);
			UnusedAssignEliminator::run(*m_context, block);
			// reverse SSA
			SSAReverser::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			UnusedPruner::run(*m_context, block);
			return block;
		}},
		{"stackCompressor", [&]() {
			auto block = disambiguate();
			updateContext(block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			size_t maxIterations = 16;
			{
				Object object(*m_optimizedObject);
				object.setCode(std::make_shared<AST>(std::get<Block>(ASTCopier{}(block))));
				block = std::get<1>(StackCompressor::run(*m_dialect, object, true, maxIterations));
			}
			BlockFlattener::run(*m_context, block);
			return block;
		}},
		{"fullSuite", [&]() {
			GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
			OptimiserSuite::run(
				*m_dialect,
				&meter,
				*m_optimizedObject,
				true,
				frontend::OptimiserSettings::DefaultYulOptimiserSteps,
				frontend::OptimiserSettings::DefaultYulOptimiserCleanupSteps,
				frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
			);
			return std::get<Block>(ASTCopier{}(m_optimizedObject->code()->root()));
		}},
		{"stackLimitEvader", [&]() {
			auto block = disambiguate();
			updateContext(block);
			Object object(*m_optimizedObject);
			object.setCode(std::make_shared<AST>(std::get<Block>(ASTCopier{}(block))));
			auto const unreachables = CompilabilityChecker{
				*m_dialect,
				object,
				true
			}.unreachableVariables;
			StackLimitEvader::run(*m_context, block, unreachables);
			return block;
		}},
		{"fakeStackLimitEvader", [&]() {
			auto block = disambiguate();
			updateContext(block);
			// Mark all variables with a name starting with "$" for escalation to memory.
			struct FakeUnreachableGenerator: ASTWalker
			{
				std::map<YulName, std::vector<YulName>> fakeUnreachables;
				using ASTWalker::operator();
				void operator()(FunctionDefinition const& _function) override
				{
					YulName originalFunctionName = m_currentFunction;
					m_currentFunction = _function.name;
					for (NameWithDebugData const& _argument: _function.parameters)
						visitVariableName(_argument.name);
					for (NameWithDebugData const& _argument: _function.returnVariables)
						visitVariableName(_argument.name);
					ASTWalker::operator()(_function);
					m_currentFunction = originalFunctionName;
				}
				void visitVariableName(YulName _var)
				{
					if (!_var.empty() && _var.str().front() == '$')
						if (!util::contains(fakeUnreachables[m_currentFunction], _var))
							fakeUnreachables[m_currentFunction].emplace_back(_var);
				}
				void operator()(VariableDeclaration const& _varDecl) override
				{
					for (auto const& var: _varDecl.variables)
						visitVariableName(var.name);
					ASTWalker::operator()(_varDecl);
				}
				void operator()(Identifier const& _identifier) override
				{
					visitVariableName(_identifier.name);
					ASTWalker::operator()(_identifier);
				}
				YulName m_currentFunction = YulName{};
			};
			FakeUnreachableGenerator fakeUnreachableGenerator;
			fakeUnreachableGenerator(block);
			StackLimitEvader::run(*m_context, block, fakeUnreachableGenerator.fakeUnreachables);
			return block;
		}}
	};
}

void YulOptimizerTestCommon::setStep(std::string const& _optimizerStep)
{
	m_optimizerStep = _optimizerStep;
}

bool YulOptimizerTestCommon::runStep()
{
	yulAssert(m_dialect, "Dialect not set.");

	if (m_namedSteps.count(m_optimizerStep))
	{
		auto block = m_namedSteps[m_optimizerStep]();
		m_optimizedObject->setCode(std::make_shared<AST>(std::move(block)));
	}
	else
		return false;

	return true;
}

std::string YulOptimizerTestCommon::randomOptimiserStep(unsigned _seed)
{
	std::mt19937 prng(_seed);
	std::uniform_int_distribution<size_t> dist(1, m_namedSteps.size());
	size_t idx = dist(prng);
	size_t count = 1;
	for (auto &step: m_namedSteps)
	{
		if (count == idx)
		{
			std::string optimiserStep = step.first;
			// Do not fuzz mainFunction
			// because it does not preserve yul code semantics.
			// Do not fuzz reasoning based simplifier because
			// it can sometimes drain memory.
			if (
				optimiserStep == "mainFunction"
			)
				// "Fullsuite" is fuzzed roughly four times more frequently than
				// other steps because of the filtering in place above.
				return "fullSuite";
			else
				return optimiserStep;
		}
		count++;
	}
	yulAssert(false, "Optimiser step selection failed.");
}

Block const* YulOptimizerTestCommon::run()
{
	return runStep() ? &m_optimizedObject->code()->root() : nullptr;
}

Block YulOptimizerTestCommon::disambiguate()
{
	auto block = std::get<Block>(Disambiguator(*m_dialect, *m_object->analysisInfo)(m_object->code()->root()));
	return block;
}

void YulOptimizerTestCommon::updateContext(Block const& _block)
{
	m_nameDispenser = std::make_unique<NameDispenser>(*m_dialect, _block, m_reservedIdentifiers);
	m_context = std::make_unique<OptimiserStepContext>(OptimiserStepContext{
		*m_dialect,
		*m_nameDispenser,
		m_reservedIdentifiers,
		frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	});
}

std::shared_ptr<Object> YulOptimizerTestCommon::optimizedObject() const
{
	return m_optimizedObject;
}
