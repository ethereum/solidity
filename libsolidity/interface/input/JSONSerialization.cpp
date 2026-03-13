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

#include <libsolidity/interface/input/JSONSerialization.h>

#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/MetadataSettings.h>

namespace solidity::frontend::input
{

void to_json(Json& _json, YulOptimizerDetails const& _details)
{
	if (_details.stackAllocation)
		_json["stackAllocation"] = *_details.stackAllocation;
	if (_details.optimizerSteps)
		_json["optimizerSteps"] = *_details.optimizerSteps;
}

void to_json(Json& _json, OptimizerDetails const& _details)
{
	if (_details.peephole)
		_json["peephole"] = *_details.peephole;
	if (_details.orderLiterals)
		_json["orderLiterals"] = *_details.orderLiterals;
	if (_details.inliner)
		_json["inliner"] = *_details.inliner;
	if (_details.jumpdestRemover)
		_json["jumpdestRemover"] = *_details.jumpdestRemover;
	if (_details.deduplicate)
		_json["deduplicate"] = *_details.deduplicate;
	if (_details.cse)
		_json["cse"] = *_details.cse;
	if (_details.constantOptimizer)
		_json["constantOptimizer"] = *_details.constantOptimizer;
	if (_details.simpleCounterForLoopUncheckedIncrement)
		_json["simpleCounterForLoopUncheckedIncrement"] = *_details.simpleCounterForLoopUncheckedIncrement;
	if (_details.yul)
		_json["yul"] = *_details.yul;
	if (_details.yulDetails && _details.yul && *_details.yul)
		_json["yulDetails"] = *_details.yulDetails;
}

void to_json(Json& _json, Optimizer const& _optimizer)
{
	if (_optimizer.enable)
		_json["enabled"] = *_optimizer.enable;
	if (_optimizer.runs)
		_json["runs"] = *_optimizer.runs;
	if (_optimizer.details)
		_json["details"] = *_optimizer.details;
}

void to_json(Json& _json, Debug const& _debug)
{
	if (_debug.revertStrings)
		_json["revertStrings"] = revertStringsToString(_debug.revertStrings.value());
	if (_debug.debugInfo)
		_json["debugInfo"] = *_debug.debugInfo;
}

void to_json(Json& _json, Metadata const& _metadata)
{
	if (_metadata.appendCBOR)
		_json["appendCBOR"] = *_metadata.appendCBOR;
	if (_metadata.bytecodeHash)
		_json["bytecodeHash"] = metadataHashToString(_metadata.bytecodeHash.value());
	if (_metadata.useLiteralContent)
		_json["useLiteralContent"] = *_metadata.useLiteralContent;
}

void to_json(Json& _json, ModelChecker const& _modelChecker)
{
	if (_modelChecker.contracts)
	{
		for (auto const& [source, contracts]: *_modelChecker.contracts)
			_json["contracts"][source] = contracts;
	}
	if (_modelChecker.divModNoSlacks)
		_json["divModNoSlacks"] = *_modelChecker.divModNoSlacks;
	if (_modelChecker.engine)
		_json["engine"] = *_modelChecker.engine;
	if (_modelChecker.extCalls)
		_json["extCalls"] = *_modelChecker.extCalls;
	if (_modelChecker.invariants)
		_json["invariants"] = *_modelChecker.invariants;
	if (_modelChecker.showProvedSafe)
		_json["showProvedSafe"] = *_modelChecker.showProvedSafe;
	if (_modelChecker.showUnproved)
		_json["showUnproved"] = *_modelChecker.showUnproved;
	if (_modelChecker.showUnsupported)
		_json["showUnsupported"] = *_modelChecker.showUnsupported;
	if (_modelChecker.solvers)
		_json["solvers"] = *_modelChecker.solvers;
	if (_modelChecker.targets)
		_json["targets"] = *_modelChecker.targets;
	if (_modelChecker.timeout)
		_json["timeout"] = *_modelChecker.timeout;
	if (_modelChecker.bmcLoopIterations)
		_json["bmcLoopIterations"] = *_modelChecker.bmcLoopIterations;
}

void to_json(Json& _json, Settings const& _settings)
{
	if (_settings.stopAfter)
		_json["stopAfter"] = *_settings.stopAfter;
	if (_settings.remappings)
		_json["remappings"] = *_settings.remappings;
	if (_settings.experimental)
		_json["experimental"] = *_settings.experimental;
	if (_settings.optimizer)
		_json["optimizer"] = *_settings.optimizer;
	if (_settings.evmVersion)
		_json["evmVersion"] = (*_settings.evmVersion).name();
	if (_settings.viaIR)
		_json["viaIR"] = *_settings.viaIR;
	if (_settings.viaSSACFG)
		_json["viaSSACFG"] = *_settings.viaSSACFG;
	if (_settings.debug)
		_json["debug"] = *_settings.debug;
	if (_settings.metadata)
		_json["metadata"] = *_settings.metadata;
	if (_settings.libraries)
		_json["libraries"] = *_settings.libraries;
	if (_settings.modelChecker)
		_json["modelChecker"] = *_settings.modelChecker;
	if (_settings.outputSelection)
		_json["outputSelection"] = *_settings.outputSelection;
}

void to_json(Json& _json, Source const& _source)
{
	if (_source.content)
		_json["content"] = *_source.content;
	if (_source.keccak256)
		_json["keccak256"] = *_source.keccak256;
	if (_source.urls)
		_json["urls"] = *_source.urls;
}

void to_json(Json& _json, Input const& _input)
{
	_json["language"] = _input.language;

	for (auto& [name, source] : _input.sources)
		_json["sources"][name] = source;

	if (_input.settings)
		_json["settings"] = *_input.settings;
}

}