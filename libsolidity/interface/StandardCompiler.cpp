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
/**
 * @author Alex Beregszaszi
 * @date 2016
 * Standard JSON compiler interface.
 */

#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libevmasm/Instruction.h>
#include <libdevcore/JSON.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value formatFatalError(string const& _type, string const& _description)
{
	Json::Value error = Json::objectValue;
	error["type"] = _type;
	error["component"] = "general";
	error["severity"] = "error";
	error["message"] = _description;

	Json::Value output = Json::objectValue;
	output["errors"] = Json::arrayValue;
	output["errors"].append(error);

	return output;
}

Json::Value StandardCompiler::compileInternal(Json::Value const& _input)
{
	m_compilerStack.reset(false);

	Json::Value const& sources = _input["sources"];
	if (!sources)
	{
		// @TODO report error
		return Json::Value();
	}

	for (auto const& sourceName: sources.getMemberNames())
		m_compilerStack.addSource(sourceName, sources[sourceName]["content"].asString());

	Json::Value const& settings = _input.get("settings", Json::Value());

	vector<string> remappings;
	for (auto const& remapping: settings.get("remappings", Json::Value()))
		remappings.push_back(remapping.asString());
	m_compilerStack.setRemappings(remappings);

	Json::Value optimizerSettings = settings.get("optimizer", Json::Value());
	bool optimize = optimizerSettings.get("enabled", Json::Value(false)).asBool();
	unsigned optimizeRuns = optimizerSettings.get("runs", Json::Value(200u)).asUInt();

	map<string, h160> libraries;
	Json::Value jsonLibraries = settings.get("libraries", Json::Value());
	for (auto const& sourceName: jsonLibraries.getMemberNames())
	{
		auto const& jsonSourceName = jsonLibraries[sourceName];
		for (auto const& library: jsonSourceName.getMemberNames())
			// @TODO use libraries only for the given source
			libraries[library] = h160(jsonSourceName[library].asString());
	}

	auto scannerFromSourceName = [&](string const& _sourceName) -> solidity::Scanner const& { return m_compilerStack.scanner(_sourceName); };

	try
	{
		// @TODO check return value and parse errors
		m_compilerStack.compile(optimize, optimizeRuns, libraries);
	}
	catch (Error const& _error)
	{
		if (_error.type() == Error::Type::DocstringParsingError)
			cerr << "Documentation parsing error: " << *boost::get_error_info<errinfo_comment>(_error) << endl;
		else
			SourceReferenceFormatter::printExceptionInformation(cerr, _error, _error.typeName(), scannerFromSourceName);

		return Json::Value();
	}
	catch (CompilerError const& _exception)
	{
		SourceReferenceFormatter::printExceptionInformation(cerr, _exception, "Compiler error", scannerFromSourceName);
		return Json::Value();
	}
	catch (InternalCompilerError const& _exception)
	{
		cerr << "Internal compiler error during compilation:" << endl
			<< boost::diagnostic_information(_exception);
		return Json::Value();
	}
	catch (UnimplementedFeatureError const& _exception)
	{
		cerr << "Unimplemented feature:" << endl
			<< boost::diagnostic_information(_exception);
		return Json::Value();
	}
	catch (Exception const& _exception)
	{
		cerr << "Exception during compilation: " << boost::diagnostic_information(_exception) << endl;
		return Json::Value();
	}
	catch (...)
	{
		cerr << "Unknown exception during compilation." << endl;
		return Json::Value();
	}

	Json::Value output = Json::objectValue;

	Json::Value contractsOutput = Json::objectValue;
	for (string const& contractName: m_compilerStack.contractNames())
	{
		// ABI, documentation and metadata
		Json::Value contractData(Json::objectValue);
		contractData["abi"] = dev::jsonCompactPrint(m_compilerStack.metadata(contractName, DocumentationType::ABIInterface));
		contractData["metadata"] = m_compilerStack.onChainMetadata(contractName);
		contractData["userdoc"] = dev::jsonCompactPrint(m_compilerStack.metadata(contractName, DocumentationType::NatspecUser));
		contractData["devdoc"] = dev::jsonCompactPrint(m_compilerStack.metadata(contractName, DocumentationType::NatspecDev));

		// EVM
		Json::Value evmData(Json::objectValue);
		// @TODO: add ir
		// @TODO: add assembly
		// @TODO: add legacyAssemblyJSON
		evmData["opcodes"] = solidity::disassemble(m_compilerStack.object(contractName).bytecode);
		// @TODO: add methodIdentifiers
		// @TODO: add gasEstimates

		// EVM bytecode
		Json::Value bytecode(Json::objectValue);
		bytecode["object"] = m_compilerStack.object(contractName).toHex();
		auto sourceMap = m_compilerStack.sourceMapping(contractName);
		bytecode["sourceMap"] = sourceMap ? *sourceMap : "";
		// @TODO: add linkReferences
		evmData["bytecode"] = bytecode;

		// EVM deployed bytecode
		Json::Value deployedBytecode(Json::objectValue);
		deployedBytecode["object"] = m_compilerStack.runtimeObject(contractName).toHex();
		auto runtimeSourceMap = m_compilerStack.runtimeSourceMapping(contractName);
		deployedBytecode["sourceMap"] = runtimeSourceMap ? *runtimeSourceMap : "";
		// @TODO: add linkReferences
		evmData["deployedBytecode"] = deployedBytecode;

		contractData["evm"] = evmData;

		contractsOutput[contractName] = contractData;
	}
	output["contracts"] = Json::objectValue;
	output["contracts"][""] = contractsOutput;

	return output;
}

Json::Value StandardCompiler::compile(Json::Value const& _input)
{
	try
	{
		return compileInternal(_input);
	}
	catch (...)
	{
		return "{\"errors\":\"[{\"type\":\"InternalCompilerError\",\"component\":\"general\",\"severity\":\"error\",\"message\":\"Internal exception in StandardCompiler::compilerInternal\"}]}";
	}
}

string StandardCompiler::compile(string const& _input)
{
	Json::Value input;

	if (!Json::Reader().parse(_input, input, false))
	{
		return "{\"errors\":\"[{\"type\":\"JSONError\",\"component\":\"general\",\"severity\":\"error\",\"message\":\"Error parsing input JSON.\"}]}";
	}

	// cout << "Input: " << input.toStyledString() << endl;
	Json::Value output = compile(input);
	// cout << "Output: " << output.toStyledString() << endl;

	try
	{
		return jsonCompactPrint(output);
	}
	catch(...)
	{
		return "{\"errors\":\"[{\"type\":\"JSONError\",\"component\":\"general\",\"severity\":\"error\",\"message\":\"Error writing output JSON.\"}]}";
	}
}
