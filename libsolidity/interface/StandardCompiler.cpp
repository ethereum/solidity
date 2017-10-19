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
#include <libdevcore/SHA3.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace {

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
	string formattedMessage = SourceReferenceFormatter::formatExceptionInformation(_exception, _type, _scannerFromSourceName);

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

set<string> requestedContractNames(Json::Value const& _outputSelection)
{
	set<string> names;
	for (auto const& sourceName: _outputSelection.getMemberNames())
	{
		for (auto const& contractName: _outputSelection[sourceName].getMemberNames())
		{
			/// Consider the "all sources" shortcuts as requesting everything.
			if (contractName == "*" || contractName == "")
				return set<string>();
			names.insert((sourceName == "*" ? "" : sourceName) + ":" + contractName);
		}
	}
	return names;
}

/// Returns true iff @a _hash (hex with 0x prefix) is the Keccak256 hash of the binary data in @a _content.
bool hashMatchesContent(string const& _hash, string const& _content)
{
	try
	{
		return dev::h256(_hash) == dev::keccak256(_content);
	}
	catch (dev::BadHexCharacter)
	{
		return false;
	}
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

bool isTargetRequired(Json::Value const& _targets, string const& _target)
{
	for (auto const& target: _targets)
		/// @TODO support sub-matching, e.g "evm" matches "evm.assembly"
		if (target == "*" || target == _target)
			return true;
	return false;
}

///
/// @a _targets is a JSON object containining a two-level hashmap, where the first level is the filename,
/// the second level is the contract name and the value is an array of target names to be requested for that contract.
/// @a _file is the current file
/// @a _contract is the current contract
/// @a _target is the current target name
///
/// @returns true if the @a _targets has a match for the requested target in the specific file / contract.
///
/// In @a _targets the use of '*' as a wildcard is permitted.
///
/// @TODO optimise this. Perhaps flatten the structure upfront.
///
bool isTargetRequired(Json::Value const& _targets, string const& _file, string const& _contract, string const& _target)
{
	if (!_targets.isObject())
		return false;

	for (auto const& file: { _file, string("*") })
		if (_targets.isMember(file) && _targets[file].isObject())
		{
			if (_contract.empty())
			{
				/// Special case for SourceUnit-level targets (such as AST)
				if (
					_targets[file].isMember("") &&
					_targets[file][""].isArray() &&
					isTargetRequired(_targets[file][""], _target)
				)
					return true;
			}
			else
			{
				/// Regular case for Contract-level targets
				for (auto const& contract: { _contract, string("*") })
					if (
						_targets[file].isMember(contract) &&
						_targets[file][contract].isArray() &&
						isTargetRequired(_targets[file][contract], _target)
					)
						return true;
			}
		}

	return false;
}

bool isTargetRequired(Json::Value const& _targets, string const& _file, string const& _contract, vector<string> const& _requested)
{
	for (auto const& requested: _requested)
		if (isTargetRequired(_targets, _file, _contract, requested))
			return true;
	return false;
}

Json::Value formatLinkReferences(std::map<size_t, std::string> const& linkReferences)
{
	Json::Value ret(Json::objectValue);

	for (auto const& ref: linkReferences)
	{
		string const& fullname = ref.second;
		size_t colon = fullname.find(':');
		solAssert(colon != string::npos, "");
		string file = fullname.substr(0, colon);
		string name = fullname.substr(colon + 1);

		Json::Value fileObject = ret.get(file, Json::objectValue);
		Json::Value libraryArray = fileObject.get(name, Json::arrayValue);

		Json::Value entry = Json::objectValue;
		entry["start"] = Json::UInt(ref.first);
		entry["length"] = 20;

		libraryArray.append(entry);
		fileObject[name] = libraryArray;
		ret[file] = fileObject;
	}

	return ret;
}

Json::Value collectEVMObject(eth::LinkerObject const& _object, string const* _sourceMap)
{
	Json::Value output = Json::objectValue;
	output["object"] = _object.toHex();
	output["opcodes"] = solidity::disassemble(_object.bytecode);
	output["sourceMap"] = _sourceMap ? *_sourceMap : "";
	output["linkReferences"] = formatLinkReferences(_object.linkReferences);
	return output;
}

}

Json::Value StandardCompiler::compileInternal(Json::Value const& _input)
{
	m_compilerStack.reset(false);

	if (!_input.isObject())
		return formatFatalError("JSONError", "Input is not a JSON object.");

	if (_input["language"] != "Solidity")
		return formatFatalError("JSONError", "Only \"Solidity\" is supported as a language.");

	Json::Value const& sources = _input["sources"];
	if (!sources)
		return formatFatalError("JSONError", "No input sources specified.");

	Json::Value errors = Json::arrayValue;

	for (auto const& sourceName: sources.getMemberNames())
	{
		string hash;

		if (!sources[sourceName].isObject())
			return formatFatalError("JSONError", "Source input is not a JSON object.");

		if (sources[sourceName]["keccak256"].isString())
			hash = sources[sourceName]["keccak256"].asString();

		if (sources[sourceName]["content"].isString())
		{
			string content = sources[sourceName]["content"].asString();
			if (!hash.empty() && !hashMatchesContent(hash, content))
				errors.append(formatError(
					false,
					"IOError",
					"general",
					"Mismatch between content and supplied hash for \"" + sourceName + "\""
				));
			else
				m_compilerStack.addSource(sourceName, content);
		}
		else if (sources[sourceName]["urls"].isArray())
		{
			if (!m_readFile)
				return formatFatalError("JSONError", "No import callback supplied, but URL is requested.");

			bool found = false;
			vector<string> failures;

			for (auto const& url: sources[sourceName]["urls"])
			{
				ReadCallback::Result result = m_readFile(url.asString());
				if (result.success)
				{
					if (!hash.empty() && !hashMatchesContent(hash, result.responseOrErrorMessage))
						errors.append(formatError(
							false,
							"IOError",
							"general",
							"Mismatch between content and supplied hash for \"" + sourceName + "\" at \"" + url.asString() + "\""
						));
					else
					{
						m_compilerStack.addSource(sourceName, result.responseOrErrorMessage);
						found = true;
						break;
					}
				}
				else
					failures.push_back("Cannot import url (\"" + url.asString() + "\"): " + result.responseOrErrorMessage);
			}

			for (auto const& failure: failures)
			{
				/// If the import succeeded, let mark all the others as warnings, otherwise all of them are errors.
				errors.append(formatError(
					found ? true : false,
					"IOError",
					"general",
					failure
				));
			}
		}
		else
			return formatFatalError("JSONError", "Invalid input source specified.");
	}

	Json::Value const& settings = _input.get("settings", Json::Value());

	vector<string> remappings;
	for (auto const& remapping: settings.get("remappings", Json::Value()))
		remappings.push_back(remapping.asString());
	m_compilerStack.setRemappings(remappings);

	Json::Value optimizerSettings = settings.get("optimizer", Json::Value());
	bool const optimize = optimizerSettings.get("enabled", Json::Value(false)).asBool();
	unsigned const optimizeRuns = optimizerSettings.get("runs", Json::Value(200u)).asUInt();
	m_compilerStack.setOptimiserSettings(optimize, optimizeRuns);

	map<string, h160> libraries;
	Json::Value jsonLibraries = settings.get("libraries", Json::Value());
	for (auto const& sourceName: jsonLibraries.getMemberNames())
	{
		auto const& jsonSourceName = jsonLibraries[sourceName];
		for (auto const& library: jsonSourceName.getMemberNames())
			// @TODO use libraries only for the given source
			libraries[library] = h160(jsonSourceName[library].asString());
	}
	m_compilerStack.setLibraries(libraries);

	Json::Value metadataSettings = settings.get("metadata", Json::Value());
	m_compilerStack.useMetadataLiteralSources(metadataSettings.get("useLiteralContent", Json::Value(false)).asBool());

	Json::Value outputSelection = settings.get("outputSelection", Json::Value());
	m_compilerStack.setRequestedContractNames(requestedContractNames(outputSelection));

	auto scannerFromSourceName = [&](string const& _sourceName) -> solidity::Scanner const& { return m_compilerStack.scanner(_sourceName); };

	try
	{
		m_compilerStack.compile();

		for (auto const& error: m_compilerStack.errors())
		{
			Error const& err = dynamic_cast<Error const&>(*error);

			errors.append(formatErrorWithException(
				*error,
				err.type() == Error::Type::Warning,
				err.typeName(),
				"general",
				"",
				scannerFromSourceName
			));
		}
	}
	/// This is only thrown in a very few locations.
	catch (Error const& _error)
	{
		errors.append(formatErrorWithException(
			_error,
			false,
			_error.typeName(),
			"general",
			"Uncaught error: ",
			scannerFromSourceName
		));
	}
	/// This should not be leaked from compile().
	catch (FatalError const& _exception)
	{
		errors.append(formatError(
			false,
			"FatalError",
			"general",
			"Uncaught fatal error: " + boost::diagnostic_information(_exception)
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
			"Internal compiler error (" + _exception.lineInfo() + ")",
			scannerFromSourceName
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
			scannerFromSourceName
		));
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

	bool const analysisSuccess = m_compilerStack.state() >= CompilerStack::State::AnalysisSuccessful;
	bool const compilationSuccess = m_compilerStack.state() == CompilerStack::State::CompilationSuccessful;

	/// Inconsistent state - stop here to receive error reports from users
	if (!compilationSuccess && (errors.size() == 0))
		return formatFatalError("InternalCompilerError", "No error reported, but compilation failed.");

	Json::Value output = Json::objectValue;

	if (errors.size() > 0)
		output["errors"] = errors;

	output["sources"] = Json::objectValue;
	unsigned sourceIndex = 0;
	for (string const& sourceName: analysisSuccess ? m_compilerStack.sourceNames() : vector<string>())
	{
		Json::Value sourceResult = Json::objectValue;
		sourceResult["id"] = sourceIndex++;
		if (isTargetRequired(outputSelection, sourceName, "", "ast"))
			sourceResult["ast"] = ASTJsonConverter(false, m_compilerStack.sourceIndices()).toJson(m_compilerStack.ast(sourceName));
		if (isTargetRequired(outputSelection, sourceName, "", "legacyAST"))
			sourceResult["legacyAST"] = ASTJsonConverter(true, m_compilerStack.sourceIndices()).toJson(m_compilerStack.ast(sourceName));
		output["sources"][sourceName] = sourceResult;
	}

	Json::Value contractsOutput = Json::objectValue;
	for (string const& contractName: compilationSuccess ? m_compilerStack.contractNames() : vector<string>())
	{
		size_t colon = contractName.find(':');
		solAssert(colon != string::npos, "");
		string file = contractName.substr(0, colon);
		string name = contractName.substr(colon + 1);

		// ABI, documentation and metadata
		Json::Value contractData(Json::objectValue);
		if (isTargetRequired(outputSelection, file, name, "abi"))
			contractData["abi"] = m_compilerStack.contractABI(contractName);
		if (isTargetRequired(outputSelection, file, name, "metadata"))
			contractData["metadata"] = m_compilerStack.metadata(contractName);
		if (isTargetRequired(outputSelection, file, name, "userdoc"))
			contractData["userdoc"] = m_compilerStack.natspecUser(contractName);
		if (isTargetRequired(outputSelection, file, name, "devdoc"))
			contractData["devdoc"] = m_compilerStack.natspecDev(contractName);

		// EVM
		Json::Value evmData(Json::objectValue);
		// @TODO: add ir
		if (isTargetRequired(outputSelection, file, name, "evm.assembly"))
			evmData["assembly"] = m_compilerStack.assemblyString(contractName, createSourceList(_input));
		if (isTargetRequired(outputSelection, file, name, "evm.legacyAssembly"))
			evmData["legacyAssembly"] = m_compilerStack.assemblyJSON(contractName, createSourceList(_input));
		if (isTargetRequired(outputSelection, file, name, "evm.methodIdentifiers"))
			evmData["methodIdentifiers"] = m_compilerStack.methodIdentifiers(contractName);
		if (isTargetRequired(outputSelection, file, name, "evm.gasEstimates"))
			evmData["gasEstimates"] = m_compilerStack.gasEstimates(contractName);

		if (isTargetRequired(
			outputSelection,
			file,
			name,
			{ "evm.bytecode", "evm.bytecode.object", "evm.bytecode.opcodes", "evm.bytecode.sourceMap", "evm.bytecode.linkReferences" }
		))
			evmData["bytecode"] = collectEVMObject(
				m_compilerStack.object(contractName),
				m_compilerStack.sourceMapping(contractName)
			);

		if (isTargetRequired(
			outputSelection,
			file,
			name,
			{ "evm.deployedBytecode", "evm.deployedBytecode.object", "evm.deployedBytecode.opcodes", "evm.deployedBytecode.sourceMap", "evm.deployedBytecode.linkReferences" }
		))
			evmData["deployedBytecode"] = collectEVMObject(
				m_compilerStack.runtimeObject(contractName),
				m_compilerStack.runtimeSourceMapping(contractName)
			);

		contractData["evm"] = evmData;

		if (!contractsOutput.isMember(file))
			contractsOutput[file] = Json::objectValue;

		contractsOutput[file][name] = contractData;
	}
	output["contracts"] = contractsOutput;

	return output;
}

Json::Value StandardCompiler::compile(Json::Value const& _input)
{
	try
	{
		return compileInternal(_input);
	}
	catch (Json::LogicError const& _exception)
	{
		return formatFatalError("InternalCompilerError", string("JSON logic exception: ") + _exception.what());
	}
	catch (Json::RuntimeError const& _exception)
	{
		return formatFatalError("InternalCompilerError", string("JSON runtime exception: ") + _exception.what());
	}
	catch (Exception const& _exception)
	{
		return formatFatalError("InternalCompilerError", "Internal exception in StandardCompiler::compileInternal: " + boost::diagnostic_information(_exception));
	}
	catch (...)
	{
		return formatFatalError("InternalCompilerError", "Internal exception in StandardCompiler::compileInternal");
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
