/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <string>
#include <iostream>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/ast/ASTPrinter.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolidity/interface/Version.h>

using namespace std;
using namespace dev;
using namespace solidity;

string formatError(Exception const& _exception, string const& _name, CompilerStack const& _compiler)
{
	ostringstream errorOutput;
	SourceReferenceFormatter::printExceptionInformation(errorOutput, _exception, _name, _compiler);
	return errorOutput.str();
}

Json::Value functionHashes(ContractDefinition const& _contract)
{
	Json::Value functionHashes(Json::objectValue);
	for (auto const& it: _contract.interfaceFunctions())
		functionHashes[it.second->externalSignature()] = toHex(it.first.ref());
	return functionHashes;
}

Json::Value gasToJson(GasEstimator::GasConsumption const& _gas)
{
	if (_gas.isInfinite || _gas.value > std::numeric_limits<Json::LargestUInt>::max())
		return Json::Value(Json::nullValue);
	else
		return Json::Value(Json::LargestUInt(_gas.value));
}

Json::Value estimateGas(CompilerStack const& _compiler, string const& _contract)
{
	eth::EVMSchedule schedule;// TODO: make relevant to supposed context.
	Json::Value gasEstimates(Json::objectValue);
	using Gas = GasEstimator::GasConsumption;
	if (!_compiler.assemblyItems(_contract) && !_compiler.runtimeAssemblyItems(_contract))
		return gasEstimates;
	if (eth::AssemblyItems const* items = _compiler.assemblyItems(_contract))
	{
		Gas gas = GasEstimator::functionalEstimation(*items);
		u256 bytecodeSize(_compiler.runtimeObject(_contract).bytecode.size());
		Json::Value creationGas(Json::arrayValue);
		creationGas[0] = gasToJson(gas);
		creationGas[1] = gasToJson(bytecodeSize * schedule.createDataGas);
		gasEstimates["creation"] = creationGas;
	}
	if (eth::AssemblyItems const* items = _compiler.runtimeAssemblyItems(_contract))
	{
		ContractDefinition const& contract = _compiler.contractDefinition(_contract);
		Json::Value externalFunctions(Json::objectValue);
		for (auto it: contract.interfaceFunctions())
		{
			string sig = it.second->externalSignature();
			externalFunctions[sig] = gasToJson(GasEstimator::functionalEstimation(*items, sig));
		}
		if (contract.fallbackFunction())
			externalFunctions[""] = gasToJson(GasEstimator::functionalEstimation(*items, "INVALID"));
		gasEstimates["external"] = externalFunctions;
		Json::Value internalFunctions(Json::objectValue);
		for (auto const& it: contract.definedFunctions())
		{
			if (it->isPartOfExternalInterface() || it->isConstructor())
				continue;
			size_t entry = _compiler.functionEntryPoint(_contract, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = GasEstimator::functionalEstimation(*items, entry, *it);
			FunctionType type(*it);
			string sig = it->name() + "(";
			auto paramTypes = type.parameterTypes();
			for (auto it = paramTypes.begin(); it != paramTypes.end(); ++it)
				sig += (*it)->toString() + (it + 1 == paramTypes.end() ? "" : ",");
			sig += ")";
			internalFunctions[sig] = gasToJson(gas);
		}
		gasEstimates["internal"] = internalFunctions;
	}
	return gasEstimates;
}

string compile(StringMap const& _sources, bool _optimize)
{
	Json::Value output(Json::objectValue);
	Json::Value errors(Json::arrayValue);
	CompilerStack compiler;
	bool success = false;
	try
	{
		compiler.addSources(_sources);
		bool succ = compiler.compile(_optimize);
		for (auto const& error: compiler.errors())
		{
			auto err = dynamic_pointer_cast<Error const>(error);
			errors.append(formatError(
				*error,
				(err->type() == Error::Type::Warning) ? "Warning" : "Error",
				compiler
			));
		}
		success = succ; // keep success false on exception
	}
	catch (Error const& error)
	{
		errors.append(formatError(error, error.typeName(), compiler));
	}
	catch (CompilerError const& exception)
	{
		errors.append(formatError(exception, "Compiler error", compiler));
	}
	catch (InternalCompilerError const& exception)
	{
		errors.append(formatError(exception, "Internal compiler error", compiler));
	}
	catch (Exception const& exception)
	{
		errors.append("Exception during compilation: " + boost::diagnostic_information(exception));
	}
	catch (...)
	{
		errors.append("Unknown exception during compilation.");
	}

	if (errors.size() > 0)
		output["errors"] = errors;

	if (success)
	{
		output["contracts"] = Json::Value(Json::objectValue);
		for (string const& contractName: compiler.contractNames())
		{
			Json::Value contractData(Json::objectValue);
			contractData["solidity_interface"] = compiler.solidityInterface(contractName);
			contractData["interface"] = compiler.interface(contractName);
			contractData["bytecode"] = compiler.object(contractName).toHex();
			contractData["runtimeBytecode"] = compiler.runtimeObject(contractName).toHex();
			contractData["opcodes"] = eth::disassemble(compiler.object(contractName).bytecode);
			contractData["functionHashes"] = functionHashes(compiler.contractDefinition(contractName));
			contractData["gasEstimates"] = estimateGas(compiler, contractName);
			ostringstream unused;
			contractData["assembly"] = compiler.streamAssembly(unused, contractName, _sources, true);
			output["contracts"][contractName] = contractData;
		}

		output["sources"] = Json::Value(Json::objectValue);
		for (auto const& source: _sources)
		{
			output["sources"][source.first] = Json::Value(Json::objectValue);
			output["sources"][source.first]["AST"] = ASTJsonConverter(compiler.ast(source.first)).json();
		}
	}

	return Json::FastWriter().write(output);
}

string compileMulti(string const& _input, bool _optimize)
{
	Json::Reader reader;
	Json::Value input;
	if (!reader.parse(_input, input, false))
	{
		Json::Value errors(Json::arrayValue);
		errors.append("Error parsing input JSON: " + reader.getFormattedErrorMessages());
		Json::Value output(Json::objectValue);
		output["errors"] = errors;
		return Json::FastWriter().write(output);
	}
	else
	{
		StringMap sources;
		Json::Value jsonSources = input["sources"];
		if (jsonSources.isObject())
			for (auto const& sourceName: jsonSources.getMemberNames())
				sources[sourceName] = jsonSources[sourceName].asString();
		return compile(sources, _optimize);
	}
}

string compileSingle(string const& _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;
	return compile(sources, _optimize);
}

static string s_outputBuffer;

extern "C"
{
extern char const* version()
{
	return VersionString.c_str();
}
extern char const* compileJSON(char const* _input, bool _optimize)
{
	s_outputBuffer = compileSingle(_input, _optimize);
	return s_outputBuffer.c_str();
}
extern char const* compileJSONMulti(char const* _input, bool _optimize)
{
	s_outputBuffer = compileMulti(_input, _optimize);
	return s_outputBuffer.c_str();
}
}
