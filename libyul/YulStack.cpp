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

#include <libyul/YulStack.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/backends/evm/EthAssemblyAdapter.h>
#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMObjectCompiler.h>
#include <libyul/ObjectParser.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/Suite.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/Ethdebug.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/algorithm/string.hpp>

#include <optional>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::util;

CharStream const& YulStack::charStream(std::string const& _sourceName) const
{
	yulAssert(m_charStream, "");
	yulAssert(m_charStream->name() == _sourceName, "");
	return *m_charStream;
}

bool YulStack::parse(std::string const& _sourceName, std::string const& _source)
{
	yulAssert(m_stackState == Empty);
	try
	{
		m_charStream = std::make_unique<CharStream>(_source, _sourceName);
		std::shared_ptr<Scanner> scanner = std::make_shared<Scanner>(*m_charStream);
		m_parserResult = ObjectParser(m_errorReporter, EVMDialect::strictAssemblyForEVMObjects(m_evmVersion)).parse(scanner, false);
	}
	catch (UnimplementedFeatureError const& _error)
	{
		reportUnimplementedFeatureError(_error);
		return false;
	}

	if (!m_errorReporter.hasErrors())
		m_stackState = Parsed;

	return m_stackState == Parsed;
}

bool YulStack::parseAndAnalyze(std::string const& _sourceName, std::string const& _source)
{
	m_errors.clear();
	yulAssert(m_stackState == Empty);

	if (!parse(_sourceName, _source))
		return false;

	yulAssert(m_stackState == Parsed);
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode());

	return analyzeParsed();
}

void YulStack::optimize(bool _viaSSACFG)
{
	yulAssert(m_stackState >= AnalysisSuccessful, "Analysis was not successful.");
	yulAssert(m_parserResult);

	try
	{
		if (
			!m_optimiserSettings.runYulOptimiser &&
			yul::MSizeFinder::containsMSize(*m_parserResult)
		)
			return;

		auto [optimizeStackAllocation, yulOptimiserSteps, yulOptimiserCleanupSteps] = [&]() -> std::tuple<bool, std::string, std::string>
		{
			if (!m_optimiserSettings.runYulOptimiser)
			{
				// Yul optimizer disabled, but empty sequence (:) explicitly provided
				if (OptimiserSuite::isEmptyOptimizerSequence(m_optimiserSettings.yulOptimiserSteps + ":" + m_optimiserSettings.yulOptimiserCleanupSteps))
					return std::make_tuple(true, "", "");
				// Yul optimizer disabled, and no sequence explicitly provided (assumes default sequence)
				else
				{
					yulAssert(
						m_optimiserSettings.yulOptimiserSteps == OptimiserSettings::DefaultYulOptimiserSteps &&
						m_optimiserSettings.yulOptimiserCleanupSteps == OptimiserSettings::DefaultYulOptimiserCleanupSteps
					);
					// Defaults are the minimum necessary to avoid running into "Stack too deep" constantly.
					return std::make_tuple(true, "u", "");
				}
			}
			return std::make_tuple(
				m_optimiserSettings.optimizeStackAllocation,
				m_optimiserSettings.yulOptimiserSteps,
				m_optimiserSettings.yulOptimiserCleanupSteps
			);
		}();

		m_stackState = Parsed;
		solAssert(m_objectOptimizer);
		m_objectOptimizer->optimize(
			*m_parserResult,
			ObjectOptimizer::Settings{
				m_evmVersion,
				optimizeStackAllocation,
				yulOptimiserSteps,
				yulOptimiserCleanupSteps,
				m_optimiserSettings.expectedExecutionsPerDeployment,
				_viaSSACFG
			}
		);

		// Optimizer does not maintain correct native source locations in the AST.
		// We can work around it by regenerating the AST from scratch from optimized IR.
		reparse();
	}
	catch (UnimplementedFeatureError const& _error)
	{
		reportUnimplementedFeatureError(_error);
	}
}

bool YulStack::analyzeParsed()
{
	yulAssert(m_stackState >= Parsed);
	yulAssert(m_parserResult, "");
	return analyzeParsed(*m_parserResult);
}

bool YulStack::analyzeParsed(Object& _object)
{
	yulAssert(m_stackState >= Parsed);
	yulAssert(_object.hasCode());
	_object.analysisInfo = std::make_shared<AsmAnalysisInfo>();

	AsmAnalyzer analyzer(
		*_object.analysisInfo,
		m_errorReporter,
		EVMDialect::strictAssemblyForEVMObjects(m_evmVersion),
		{},
		_object.summarizeStructure()
	);

	bool success = false;
	try
	{
		success = analyzer.analyze(_object.code()->root());
		for (auto& subNode: _object.subObjects)
			if (auto subObject = dynamic_cast<Object*>(subNode.get()))
				if (!analyzeParsed(*subObject))
					success = false;
	}
	catch (UnimplementedFeatureError const& _error)
	{
		reportUnimplementedFeatureError(_error);
		success = false;
	}

	if (success)
		m_stackState = AnalysisSuccessful;

	return success;
}

void YulStack::compileEVM(AbstractAssembly& _assembly, bool _optimize, bool _viaSSACFG) const
{
	EVMObjectCompiler::compile(*m_parserResult, _assembly, _optimize, _viaSSACFG);
}

void YulStack::reparse()
{
	yulAssert(m_parserResult);
	yulAssert(m_charStream);

	// NOTE: it is important for the source printed here to exactly match what the compiler will
	// eventually output to the user. In particular, debug info must be exactly the same.
	// Otherwise source locations will be off.
	std::string source = print();

	YulStack cleanStack(
		m_evmVersion,
		m_optimiserSettings,
		m_debugInfoSelection,
		m_soliditySourceProvider,
		m_objectOptimizer
	);
	bool reanalysisSuccessful = cleanStack.parseAndAnalyze(m_charStream->name(), source);
	yulAssert(
		reanalysisSuccessful,
		source + "\n\n"
		"Invalid IR generated:\n" +
		SourceReferenceFormatter::formatErrorInformation(cleanStack.errors(), cleanStack) + "\n"
	);

	m_stackState = AnalysisSuccessful;
	m_parserResult = std::move(cleanStack.m_parserResult);

	// NOTE: We keep the char stream, and errors, even though they no longer match the object,
	// because it's the original source that matters to the user. Optimized code may have different
	// locations and fewer warnings.
}

MachineAssemblyObject YulStack::assemble(Machine _machine, bool _viaSSACFG)
{
	yulAssert(m_stackState >= AnalysisSuccessful);
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode(), "");
	yulAssert(m_parserResult->analysisInfo, "");

	switch (_machine)
	{
	case Machine::EVM:
		return assembleWithDeployed({}, _viaSSACFG).first;
	}
	unreachable();
}

std::pair<MachineAssemblyObject, MachineAssemblyObject>
YulStack::assembleWithDeployed(std::optional<std::string_view> _deployName, bool _viaSSACFG)
{
	yulAssert(m_charStream);

	auto [creationAssembly, deployedAssembly] = assembleEVMWithDeployed(_deployName, _viaSSACFG);
	if (!creationAssembly)
	{
		yulAssert(!deployedAssembly);
		return {MachineAssemblyObject{}, MachineAssemblyObject{}};
	}

	MachineAssemblyObject creationObject;
	MachineAssemblyObject deployedObject;
	try
	{
		creationObject.bytecode = std::make_shared<evmasm::LinkerObject>(creationAssembly->assemble());
		yulAssert(creationObject.bytecode->immutableReferences.empty(), "Leftover immutables.");
		creationObject.assembly = creationAssembly;
		creationObject.sourceMappings = std::make_unique<std::string>();
		*creationObject.sourceMappings = evmasm::AssemblyItem::computeSourceMapping(
			creationAssembly->items(),
			{{m_charStream->name(), 0}}
		);
		if (debugInfoSelection().ethdebug)
			creationObject.ethdebug = evmasm::ethdebug::program(creationObject.assembly->name(), 0, *creationObject.assembly, *creationObject.bytecode);

		if (deployedAssembly)
		{
			deployedObject.bytecode = std::make_shared<evmasm::LinkerObject>(deployedAssembly->assemble());
			deployedObject.assembly = deployedAssembly;
			if (debugInfoSelection().ethdebug)
				deployedObject.ethdebug = evmasm::ethdebug::program(deployedObject.assembly->name(), 0, *deployedObject.assembly, *deployedObject.bytecode);
			deployedObject.sourceMappings = std::make_unique<std::string>(
				evmasm::AssemblyItem::computeSourceMapping(
					deployedAssembly->items(),
					{{m_charStream->name(), 0}}
				)
			);
		}
	}
	catch (Error const& _error)
	{
		m_errorReporter.codeGenerationError(_error);
		return {MachineAssemblyObject{}, MachineAssemblyObject{}};
	}
	catch (UnimplementedFeatureError const& _error)
	{
		reportUnimplementedFeatureError(_error);
		return {MachineAssemblyObject{}, MachineAssemblyObject{}};
	}

	return {std::move(creationObject), std::move(deployedObject)};
}

std::pair<std::shared_ptr<evmasm::Assembly>, std::shared_ptr<evmasm::Assembly>>
YulStack::assembleEVMWithDeployed(std::optional<std::string_view> _deployName, bool _viaSSACFG)
{
	yulAssert(m_stackState >= AnalysisSuccessful);
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode(), "");
	yulAssert(m_parserResult->analysisInfo, "");

	evmasm::Assembly assembly(m_evmVersion, true, {});
	EthAssemblyAdapter adapter(assembly);

	// NOTE: We always need stack optimization when Yul optimizer is disabled (unless code contains
	// msize). It being disabled just means that we don't use the full step sequence. We still run
	// it with the minimal steps required to avoid "stack too deep".
	bool optimize = m_optimiserSettings.optimizeStackAllocation || (
		!m_optimiserSettings.runYulOptimiser &&
		!yul::MSizeFinder::containsMSize(*m_parserResult)
	);
	try
	{
		compileEVM(adapter, optimize, _viaSSACFG);

		assembly.optimise(evmasm::Assembly::OptimiserSettings::translateSettings(m_optimiserSettings));

		std::optional<evmasm::SubAssemblyID> subIndex;

		// Pick matching assembly if name was given
		if (_deployName.has_value())
		{
			for (size_t i = 0; i < assembly.numSubs(); i++)
				if (assembly.sub({i}).name() == _deployName)
				{
					subIndex = {i};
					break;
				}

			solAssert(subIndex.has_value(), "Failed to find object to be deployed.");
		}
		// Otherwise use heuristic: If there is a single sub-assembly, this is likely the object to be deployed.
		else if (assembly.numSubs() == 1)
			subIndex = {0};

		if (subIndex.has_value())
		{
			evmasm::Assembly& runtimeAssembly = assembly.sub(*subIndex);
			return {std::make_shared<evmasm::Assembly>(assembly), std::make_shared<evmasm::Assembly>(runtimeAssembly)};
		}
	}
	catch (Error const& _error)
	{
		m_errorReporter.codeGenerationError(_error);
		return {nullptr, nullptr};
	}
	catch (UnimplementedFeatureError const& _error)
	{
		reportUnimplementedFeatureError(_error);
		return {nullptr, nullptr};
	}

	return {std::make_shared<evmasm::Assembly>(assembly), nullptr};
}

std::string YulStack::print() const
{
	yulAssert(m_stackState >= Parsed);
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode(), "");
	return (m_debugInfoSelection.ethdebug ? "/// ethdebug: enabled\n" : "") + m_parserResult->toString(
		m_debugInfoSelection,
		m_soliditySourceProvider
	) + "\n";
}

Json YulStack::astJson() const
{
	yulAssert(m_stackState >= Parsed);
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode(), "");
	return  m_parserResult->toJson();
}

std::shared_ptr<Object> YulStack::parserResult() const
{
	yulAssert(m_stackState >= AnalysisSuccessful, "Analysis was not successful.");
	yulAssert(m_parserResult, "");
	yulAssert(m_parserResult->hasCode(), "");
	return m_parserResult;
}

Dialect const& YulStack::dialect() const
{
	yulAssert(m_stackState >= AnalysisSuccessful);
	yulAssert(m_parserResult && m_parserResult->dialect());
	return *m_parserResult->dialect();
}

void YulStack::reportUnimplementedFeatureError(UnimplementedFeatureError const& _error)
{
	yulAssert(m_charStream);
	yulAssert(_error.comment(), "Errors must include a message for the user.");
	if (_error.sourceLocation().sourceName)
		yulAssert(*_error.sourceLocation().sourceName == m_charStream->name());
	m_errorReporter.unimplementedFeatureError(1920_error, _error.sourceLocation(), *_error.comment());
}
