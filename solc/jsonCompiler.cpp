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
#include <libevmcore/Params.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/SourceReferenceFormatter.h>
#include <libsolidity/ASTJsonConverter.h>

using namespace std;
using namespace dev;
using namespace solidity;

string formatError(Exception const& _exception, string const& _name, CompilerStack const& _compiler)
{
	ostringstream errorOutput;
	SourceReferenceFormatter::printExceptionInformation(errorOutput, _exception, _name, _compiler);

	Json::Value output(Json::objectValue);
	output["error"] = errorOutput.str();
	return Json::FastWriter().write(output);
}

Json::Value functionHashes(ContractDefinition const& _contract)
{
	Json::Value functionHashes(Json::objectValue);
	for (auto const& it: _contract.getInterfaceFunctions())
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
	Json::Value gasEstimates(Json::objectValue);
	using Gas = GasEstimator::GasConsumption;
	if (!_compiler.getAssemblyItems(_contract) && !_compiler.getRuntimeAssemblyItems(_contract))
		return gasEstimates;
	if (eth::AssemblyItems const* items = _compiler.getAssemblyItems(_contract))
	{
		Gas gas = GasEstimator::functionalEstimation(*items);
		u256 bytecodeSize(_compiler.getRuntimeBytecode(_contract).size());
		Json::Value creationGas(Json::arrayValue);
		creationGas[0] = gasToJson(gas);
		creationGas[1] = gasToJson(bytecodeSize * eth::c_createDataGas);
		gasEstimates["creation"] = creationGas;
	}
	if (eth::AssemblyItems const* items = _compiler.getRuntimeAssemblyItems(_contract))
	{
		ContractDefinition const& contract = _compiler.getContractDefinition(_contract);
		Json::Value externalFunctions(Json::objectValue);
		for (auto it: contract.getInterfaceFunctions())
		{
			string sig = it.second->externalSignature();
			externalFunctions[sig] = gasToJson(GasEstimator::functionalEstimation(*items, sig));
		}
		if (contract.getFallbackFunction())
			externalFunctions[""] = gasToJson(GasEstimator::functionalEstimation(*items, "INVALID"));
		gasEstimates["external"] = externalFunctions;
		Json::Value internalFunctions(Json::objectValue);
		for (auto const& it: contract.getDefinedFunctions())
		{
			if (it->isPartOfExternalInterface() || it->isConstructor())
				continue;
			size_t entry = _compiler.getFunctionEntryPoint(_contract, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = GasEstimator::functionalEstimation(*items, entry, *it);
			FunctionType type(*it);
			string sig = it->getName() + "(";
			auto end = type.getParameterTypes().end();
			for (auto it = type.getParameterTypes().begin(); it != end; ++it)
				sig += (*it)->toString() + (it + 1 == end ? "" : ",");
			sig += ")";
			internalFunctions[sig] = gasToJson(gas);
		}
		gasEstimates["internal"] = internalFunctions;
	}
	return gasEstimates;
}

string compile(string _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;

	Json::Value output(Json::objectValue);
	CompilerStack compiler;
	try
	{
		compiler.compile(_input, _optimize);
	}
	catch (ParserError const& exception)
	{
		return formatError(exception, "Parser error", compiler);
	}
	catch (DeclarationError const& exception)
	{
		return formatError(exception, "Declaration error", compiler);
	}
	catch (TypeError const& exception)
	{
		return formatError(exception, "Type error", compiler);
	}
	catch (CompilerError const& exception)
	{
		return formatError(exception, "Compiler error", compiler);
	}
	catch (InternalCompilerError const& exception)
	{
		return formatError(exception, "Internal compiler error", compiler);
	}
	catch (DocstringParsingError const& exception)
	{
		return formatError(exception, "Documentation parsing error", compiler);
	}
	catch (Exception const& exception)
	{
		output["error"] = "Exception during compilation: " + boost::diagnostic_information(exception);
		return Json::FastWriter().write(output);
	}
	catch (...)
	{
		output["error"] = "Unknown exception during compilation.";
		return Json::FastWriter().write(output);
	}

	output["contracts"] = Json::Value(Json::objectValue);
	for (string const& contractName: compiler.getContractNames())
	{
		Json::Value contractData(Json::objectValue);
		contractData["solidity_interface"] = compiler.getSolidityInterface(contractName);
		contractData["interface"] = compiler.getInterface(contractName);
		contractData["bytecode"] = toHex(compiler.getBytecode(contractName));
		contractData["opcodes"] = eth::disassemble(compiler.getBytecode(contractName));
		contractData["functionHashes"] = functionHashes(compiler.getContractDefinition(contractName));
		contractData["gasEstimates"] = estimateGas(compiler, contractName);
		ostringstream unused;
		contractData["assembly"] = compiler.streamAssembly(unused, contractName, sources, true);
		output["contracts"][contractName] = contractData;
	}

	output["sources"] = Json::Value(Json::objectValue);
	output["sources"][""] = Json::Value(Json::objectValue);
	output["sources"][""]["AST"] = ASTJsonConverter(compiler.getAST("")).json();

	return Json::FastWriter().write(output);
}

static string outputBuffer;

extern "C"
{
extern char const* compileJSON(char const* _input, bool _optimize)
{
	outputBuffer = compile(_input, _optimize);
	return outputBuffer.c_str();
}
}
