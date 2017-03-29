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
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libevmasm/Instruction.h>
#include <libdevcore/JSON.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value formatError(
	bool _warning,
	string const& _type,
	string const& _component,
	string const& _message,
	string const& _formattedMessage = "",
	Json::Value const& _sourceLocation = Json::Value()
)
{
	Json::Value error = Json::objectValue;
	error["type"] = _type;
	error["component"] = _component;
	error["severity"] = _warning ? "warning" : "error";
	error["message"] = _message;
	error["formattedMessage"] = (_formattedMessage.length() > 0) ? _formattedMessage : _message;
	if (_sourceLocation.isObject())
		error["sourceLocation"] = _sourceLocation;
	return error;
}

Json::Value formatFatalError(string const& _type, string const& _message)
{
	Json::Value output = Json::objectValue;
	output["errors"] = Json::arrayValue;
	output["errors"].append(formatError(false, _type, "general", _message));
	return output;
}

Json::Value formatErrorWithException(
	Exception const& _exception,
	bool const& _warning,
	string const& _type,
	string const& _component,
	string const& _message,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	string message;
	string formattedMessage = SourceReferenceFormatter::formatExceptionInformation(_exception, _message, _scannerFromSourceName);

	// NOTE: the below is partially a copy from SourceReferenceFormatter
	SourceLocation const* location = boost::get_error_info<errinfo_sourceLocation>(_exception);

	if (string const* description = boost::get_error_info<errinfo_comment>(_exception))
		message = ((_message.length() > 0) ? (_message + ":") : "") + *description;
	else
		message = _message;

	if (location && location->sourceName)
	{
		Json::Value sourceLocation = Json::objectValue;
		sourceLocation["file"] = *location->sourceName;
		sourceLocation["start"] = location->start;
		sourceLocation["end"] = location->end;
	}

	return formatError(_warning, _type, _component, message, formattedMessage, location);
}

StringMap createSourceList(Json::Value const& _input)
{
	StringMap sources;
	Json::Value const& jsonSources = _input["sources"];
	if (jsonSources.isObject())
		for (auto const& sourceName: jsonSources.getMemberNames())
			sources[sourceName] = jsonSources[sourceName]["content"].asString();
	return sources;
}

Json::Value StandardCompiler::compileInternal(Json::Value const& _input)
{
	m_compilerStack.reset(false);

	Json::Value const& sources = _input["sources"];
	if (!sources)
		return formatFatalError("JSONError", "No input sources specified.");

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

	Json::Value errors = Json::arrayValue;
	bool success = false;

	try
	{
		success = m_compilerStack.compile(optimize, optimizeRuns, libraries);

		for (auto const& error: m_compilerStack.errors())
		{
			auto err = dynamic_pointer_cast<Error const>(error);

			errors.append(formatErrorWithException(
				*error,
				err->type() == Error::Type::Warning,
				err->typeName(),
				"general",
				"",
				scannerFromSourceName
			));
		}
	}
	catch (Error const& _error)
	{
		if (_error.type() == Error::Type::DocstringParsingError)
			errors.append(formatError(
				false,
				"DocstringParsingError",
				"general",
				"Documentation parsing error: " + *boost::get_error_info<errinfo_comment>(_error)
			));
		else
			errors.append(formatErrorWithException(
				_error,
				false,
				_error.typeName(),
				"general",
				"",
				scannerFromSourceName
			));
	}
	catch (CompilerError const& _exception)
	{
		errors.append(formatErrorWithException(
			_exception,
			false,
			"CompilerError",
			"general",
			"Compiler error (" + _exception.lineInfo() + ")",
			scannerFromSourceName
		));
	}
	catch (InternalCompilerError const& _exception)
	{
		errors.append(formatErrorWithException(
			_exception,
			false,
			"InternalCompilerError",
			"general",
			"Internal compiler error (" + _exception.lineInfo() + ")", scannerFromSourceName
		));
	}
	catch (UnimplementedFeatureError const& _exception)
	{
		errors.append(formatErrorWithException(
			_exception,
			false,
			"UnimplementedFeatureError",
			"general",
			"Unimplemented feature (" + _exception.lineInfo() + ")",
			scannerFromSourceName));
	}
	catch (Exception const& _exception)
	{
		errors.append(formatError(
			false,
			"Exception",
			"general",
			"Exception during compilation: " + boost::diagnostic_information(_exception)
		));
	}
	catch (...)
	{
		errors.append(formatError(
			false,
			"Exception",
			"general",
			"Unknown exception during compilation."
		));
	}

	Json::Value output = Json::objectValue;

	if (errors.size() > 0)
		output["errors"] = errors;

	/// Inconsistent state - stop here to receive error reports from users
	if (!success && (errors.size() == 0))
		return formatFatalError("InternalCompilerError", "No error reported, but compilation failed.");

	output["sources"] = Json::objectValue;
	unsigned sourceIndex = 0;
	for (auto const& source: m_compilerStack.sourceNames())
	{
		Json::Value sourceResult = Json::objectValue;
		sourceResult["id"] = sourceIndex++;
		sourceResult["ast"] = ASTJsonConverter(m_compilerStack.ast(source), m_compilerStack.sourceIndices()).json();
		output["sources"][source] = sourceResult;
	}

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
		ostringstream unused;
		evmData["legacyAssembly"] = m_compilerStack.streamAssembly(unused, contractName, createSourceList(_input), true);
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
		return formatFatalError("InternalCompilerError", "Internal exception in StandardCompiler::compilerInternal");
	}
}

string StandardCompiler::compile(string const& _input)
{
	Json::Value input;
	Json::Reader reader;

	try
	{
		if (!reader.parse(_input, input, false))
			return jsonCompactPrint(formatFatalError("JSONError", reader.getFormattedErrorMessages()));
	}
	catch(...)
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
