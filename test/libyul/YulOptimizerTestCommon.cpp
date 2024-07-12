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
	std::shared_ptr<Object> _obj
): m_object(_obj), m_resultObject(std::make_shared<Object>()), m_analysisInfo(m_object->analysisInfo)
{
	*m_resultObject = *m_object;
	m_namedSteps = {
		{"disambiguator", [&](YulNameRepository& _nameRepository) {
			 return disambiguate(_nameRepository);
		}},
		{"nameDisplacer", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			NameDisplacer{
				*m_nameDispenser,
				_nameRepository,
				{"illegal1"_yulname, "illegal2"_yulname, "illegal3"_yulname, "illegal4"_yulname, "illegal5"_yulname}
			}(block);
			return block;
		}},
		{"blockFlattener", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionGrouper::run(*m_context, block);
			BlockFlattener::run(*m_context, block);
			return block;
		}},
		{"constantOptimiser", [&](YulNameRepository& _nameRepository) {
			auto block = std::get<Block>(ASTCopier{}(m_object->code->block()));
			updateContext(_nameRepository, block);
			GasMeter meter(_nameRepository, false, 200);
			ConstantOptimiser{_nameRepository, meter}(block);
			return block;
		}},
		{"varDeclInitializer", [&](YulNameRepository& _nameRepository) {
			auto block = std::get<Block>(ASTCopier{}(m_object->code->block()));
			updateContext(_nameRepository, block);
			VarDeclInitializer::run(*m_context, block);
			return block;
		}},
		{"varNameCleaner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			VarNameCleaner::run(*m_context, block);
			return block;
		}},
		{"forLoopConditionIntoBody", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopConditionIntoBody::run(*m_context, block);
			return block;
		}},
		{"forLoopInitRewriter", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			return block;
		}},
		{"commonSubexpressionEliminator", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			CommonSubexpressionEliminator::run(*m_context, block);
			return block;
		}},
		{"conditionalUnsimplifier", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ConditionalUnsimplifier::run(*m_context, block);
			return block;
		}},
		{"conditionalSimplifier", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ConditionalSimplifier::run(*m_context, block);
			return block;
		}},
		{"expressionSplitter", [&](YulNameRepository& _nameRepository) {
			auto block = std::get<Block>(ASTCopier{}(m_object->code->block()));
			updateContext(_nameRepository, block);
			ExpressionSplitter::run(*m_context, block);
			return block;
		}},
		{"expressionJoiner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"splitJoin", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ExpressionSplitter::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"functionGrouper", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionGrouper::run(*m_context, block);
			return block;
		}},
		{"functionHoister", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			return block;
		}},
		{"functionSpecializer", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			FunctionSpecializer::run(*m_context, block);
			return block;
		}},
		{"expressionInliner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ExpressionInliner::run(*m_context, block);
			return block;
		}},
		{"fullInliner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			FullInliner::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"fullInlinerWithoutSplitter", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			FullInliner::run(*m_context, block);
			return block;
		}},
		{"rematerialiser", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			Rematerialiser::run(*m_context, block);
			return block;
		}},
		{"expressionSimplifier", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
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
		{"fullSimplify", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
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
		{"unusedFunctionParameterPruner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LiteralRematerialiser::run(*m_context, block);
			UnusedFunctionParameterPruner::run(*m_context, block);
			return block;
		}},
		{"unusedPruner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			UnusedPruner::run(*m_context, block);
			return block;
		}},
		{"circularReferencesPruner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			CircularReferencesPruner::run(*m_context, block);
			return block;
		}},
		{"deadCodeEliminator", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			DeadCodeEliminator::run(*m_context, block);
			return block;
		}},
		{"ssaTransform", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			return block;
		}},
		{"unusedAssignEliminator", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			UnusedAssignEliminator::run(*m_context, block);
			return block;
		}},
		{"unusedStoreEliminator", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			ExpressionSplitter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			UnusedStoreEliminator::run(*m_context, block);
			SSAReverser::run(*m_context, block);
			ExpressionJoiner::run(*m_context, block);
			return block;
		}},
		{"equalStoreEliminator", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			FunctionHoister::run(*m_context, block);
			ForLoopInitRewriter::run(*m_context, block);
			EqualStoreEliminator::run(*m_context, block);
			return block;
		}},
		{"ssaPlusCleanup", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			SSATransform::run(*m_context, block);
			UnusedAssignEliminator::run(*m_context, block);
			return block;
		}},
		{"loadResolver", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
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
		{"loopInvariantCodeMotion", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LoopInvariantCodeMotion::run(*m_context, block);
			return block;
		}},
		{"controlFlowSimplifier", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			ControlFlowSimplifier::run(*m_context, block);
			return block;
		}},
		{"structuralSimplifier", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			LiteralRematerialiser::run(*m_context, block);
			StructuralSimplifier::run(*m_context, block);
			return block;
		}},
		{"equivalentFunctionCombiner", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			EquivalentFunctionCombiner::run(*m_context, block);
			return block;
		}},
		{"ssaReverser", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			SSAReverser::run(*m_context, block);
			return block;
		}},
		{"ssaAndBack", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
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
		{"stackCompressor", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			ForLoopInitRewriter::run(*m_context, block);
			FunctionHoister::run(*m_context, block);
			FunctionGrouper::run(*m_context, block);
			size_t maxIterations = 16;
			StackCompressor::run(_nameRepository, block, *m_object, true, maxIterations);
			BlockFlattener::run(*m_context, block);
			return block;
		}},
		{"fullSuite", [&](YulNameRepository& _nameRepository) {
			GasMeter meter(_nameRepository, false, 200);
			OptimiserSuite::run(
				&meter,
				*m_resultObject,
				true,
				frontend::OptimiserSettings::DefaultYulOptimiserSteps,
				frontend::OptimiserSettings::DefaultYulOptimiserCleanupSteps,
				frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
			);
			return std::get<Block>(ASTCopier{}(m_resultObject->code->block()));
		}},
		{"stackLimitEvader", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			StackLimitEvader::run(*m_context, block, CompilabilityChecker{
				*m_object,
				true,
				&_nameRepository,
				&block
			}.unreachableVariables);
			return block;
		}},
		{"fakeStackLimitEvader", [&](YulNameRepository& _nameRepository) {
			auto block = disambiguate(_nameRepository);
			updateContext(_nameRepository, block);
			// Mark all variables with a name starting with "$" for escalation to memory.
			struct FakeUnreachableGenerator: ASTWalker
			{
				std::map<YulName, std::vector<YulName>> fakeUnreachables;
				using ASTWalker::operator();
				void operator()(FunctionDefinition const& _function) override
				{
					YulName originalFunctionName = m_currentFunction;
					m_currentFunction = _function.name;
					for (TypedName const& _argument: _function.parameters)
						visitVariableName(_argument.name);
					for (TypedName const& _argument: _function.returnVariables)
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
	yulAssert(m_object, "Object not set.");
	yulAssert(m_object->code, "Code of object not set.");
	if (m_namedSteps.count(m_optimizerStep))
	{
		YulNameRepository nameRepository(m_object->code->nameRepository());
		auto block = m_namedSteps[m_optimizerStep](nameRepository);
		m_resultObject->code = std::make_shared<AST>(std::move(nameRepository), std::move(block));
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
	return runStep() ? &m_resultObject->code->block() : nullptr;
}

Block YulOptimizerTestCommon::disambiguate(YulNameRepository& _nameRepository)
{
	auto block = std::get<Block>(Disambiguator(_nameRepository, *m_analysisInfo)(m_object->code->block()));
	m_analysisInfo.reset();
	return block;
}

void YulOptimizerTestCommon::updateContext(YulNameRepository& _nameRepository, Block const& _block)
{
	m_nameDispenser = std::make_unique<NameDispenser>(_nameRepository.dialect(), _block, m_reservedIdentifiers);
	m_context = std::make_unique<OptimiserStepContext>(OptimiserStepContext{
		_nameRepository.dialect(),
		_nameRepository,
		*m_nameDispenser,
		m_reservedIdentifiers,
		frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	});
}

std::shared_ptr<Object> YulOptimizerTestCommon::resultObject() const
{
	return m_resultObject;
}
