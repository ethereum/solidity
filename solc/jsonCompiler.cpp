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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <string>
#include <functional>
#include <iostream>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/JSON.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/GasMeter.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/ast/ASTPrinter.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolidity/interface/Version.h>

using namespace std;
using namespace dev;
using namespace solidity;

extern "C" {
/// Callback used to retrieve additional source files. "Returns" two pointers that should be
/// heap-allocated and are free'd by the caller.
typedef void (*CStyleReadFileCallback)(char const* _path, char** o_contents, char** o_error);
}

ReadFile::Callback wrapReadCallback(CStyleReadFileCallback _readCallback = nullptr)
{
	ReadFile::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _path)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_path.c_str(), &contents_c, &error_c);
			ReadFile::Result result;
			result.success = true;
			if (!contents_c && !error_c)
			{
				result.success = false;
				result.contentsOrErrorMessage = "File not found.";
			}
			if (contents_c)
			{
				result.success = true;
				result.contentsOrErrorMessage = string(contents_c);
				free(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.contentsOrErrorMessage = string(error_c);
				free(error_c);
			}
			return result;
		};
	}
	return readCallback;
}

Json::Value functionHashes(ContractDefinition const& _contract)
{
	Json::Value functionHashes(Json::objectValue);
	for (auto const& it: _contract.interfaceFunctions())
		functionHashes[it.second->externalSignature()] = toHex(it.first.ref());
	return functionHashes;
}

/// Translates a gas value as a string to a JSON number or null
Json::Value gasToJson(Json::Value const& _value)
{
	if (_value.isObject())
	{
		Json::Value ret = Json::objectValue;
		for (auto const& sig: _value.getMemberNames())
			ret[sig] = gasToJson(_value[sig]);
		return ret;
	}

	if (_value == "infinite")
		return Json::Value(Json::nullValue);

	u256 value(_value.asString());
	if (value > std::numeric_limits<Json::LargestUInt>::max())
		return Json::Value(Json::nullValue);
	else
		return Json::Value(Json::LargestUInt(value));
}

Json::Value estimateGas(CompilerStack const& _compiler, string const& _contract)
{
	Json::Value estimates = _compiler.gasEstimates(_contract);
	Json::Value output(Json::objectValue);

	if (estimates["creation"].isObject())
	{
		Json::Value creation(Json::arrayValue);
		creation[0] = gasToJson(estimates["creation"]["executionCost"]);
		creation[1] = gasToJson(estimates["creation"]["codeDepositCost"]);
		output["creation"] = creation;
	}
	else
		output["creation"] = Json::objectValue;
	output["external"] = gasToJson(estimates.get("external", Json::objectValue));
	output["internal"] = gasToJson(estimates.get("internal", Json::objectValue));

	return output;
}

string compile(StringMap const& _sources, bool _optimize, CStyleReadFileCallback _readCallback)
{
	Json::Value output(Json::objectValue);
	Json::Value errors(Json::arrayValue);
	CompilerStack compiler(wrapReadCallback(_readCallback));
	auto scannerFromSourceName = [&](string const& _sourceName) -> solidity::Scanner const& { return compiler.scanner(_sourceName); };
	bool success = false;
	try
	{
		compiler.addSources(_sources);
		bool succ = compiler.compile(_optimize);
		for (auto const& error: compiler.errors())
		{
			auto err = dynamic_pointer_cast<Error const>(error);
			errors.append(SourceReferenceFormatter::formatExceptionInformation(
				*error,
				(err->type() == Error::Type::Warning) ? "Warning" : "Error",
				scannerFromSourceName
			));
		}
		success = succ; // keep success false on exception
	}
	catch (Error const& error)
	{
		errors.append(SourceReferenceFormatter::formatExceptionInformation(error, error.typeName(), scannerFromSourceName));
	}
	catch (CompilerError const& exception)
	{
		errors.append(SourceReferenceFormatter::formatExceptionInformation(exception, "Compiler error (" + exception.lineInfo() + ")", scannerFromSourceName));
	}
	catch (InternalCompilerError const& exception)
	{
		errors.append(SourceReferenceFormatter::formatExceptionInformation(exception, "Internal compiler error (" + exception.lineInfo() + ")", scannerFromSourceName));
	}
	catch (UnimplementedFeatureError const& exception)
	{
		errors.append(SourceReferenceFormatter::formatExceptionInformation(exception, "Unimplemented feature (" + exception.lineInfo() + ")", scannerFromSourceName));
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
		try
		{
			output["contracts"] = Json::Value(Json::objectValue);
			for (string const& contractName: compiler.contractNames())
			{
				Json::Value contractData(Json::objectValue);
				contractData["interface"] = dev::jsonCompactPrint(compiler.contractABI(contractName));
				contractData["bytecode"] = compiler.object(contractName).toHex();
				contractData["runtimeBytecode"] = compiler.runtimeObject(contractName).toHex();
				contractData["opcodes"] = solidity::disassemble(compiler.object(contractName).bytecode);
				contractData["metadata"] = compiler.onChainMetadata(contractName);
				contractData["functionHashes"] = functionHashes(compiler.contractDefinition(contractName));
				contractData["gasEstimates"] = estimateGas(compiler, contractName);
				auto sourceMap = compiler.sourceMapping(contractName);
				contractData["srcmap"] = sourceMap ? *sourceMap : "";
				auto runtimeSourceMap = compiler.runtimeSourceMapping(contractName);
				contractData["srcmapRuntime"] = runtimeSourceMap ? *runtimeSourceMap : "";
				ostringstream unused;
				contractData["assembly"] = compiler.streamAssembly(unused, contractName, _sources, true);
				output["contracts"][contractName] = contractData;
			}
		}
		catch (...)
		{
			output["errors"].append("Unknown exception while generating contract data output.");
		}

		try
		{
			// Do not taint the internal error list
			ErrorList formalErrors;
			if (compiler.prepareFormalAnalysis(&formalErrors))
				output["formal"]["why3"] = compiler.formalTranslation();
			if (!formalErrors.empty())
			{
				Json::Value errors(Json::arrayValue);
				for (auto const& error: formalErrors)
					errors.append(SourceReferenceFormatter::formatExceptionInformation(
						*error,
						(error->type() == Error::Type::Warning) ? "Warning" : "Error",
						scannerFromSourceName
					));
				output["formal"]["errors"] = errors;
			}
		}
		catch (...)
		{
			output["errors"].append("Unknown exception while generating formal method output.");
		}

		try
		{
			// Indices into this array are used to abbreviate source names in source locations.
			output["sourceList"] = Json::Value(Json::arrayValue);
			for (auto const& source: compiler.sourceNames())
				output["sourceList"].append(source);
			output["sources"] = Json::Value(Json::objectValue);
			for (auto const& source: compiler.sourceNames())
				output["sources"][source]["AST"] = ASTJsonConverter(true, compiler.sourceIndices()).toJson(compiler.ast(source));
		}
		catch (...)
		{
			output["errors"].append("Unknown exception while generating source name output.");
		}
	}

	try
	{
		return dev::jsonCompactPrint(output);
	}
	catch (...)
	{
		return "{\"errors\":[\"Unknown error while generating JSON.\"]}";
	}
}

string compileMulti(string const& _input, bool _optimize, CStyleReadFileCallback _readCallback = nullptr)
{
	Json::Reader reader;
	Json::Value input;
	if (!reader.parse(_input, input, false))
	{
		Json::Value errors(Json::arrayValue);
		errors.append("Error parsing input JSON: " + reader.getFormattedErrorMessages());
		Json::Value output(Json::objectValue);
		output["errors"] = errors;
		return dev::jsonCompactPrint(output);
	}
	else
	{
		StringMap sources;
		Json::Value jsonSources = input["sources"];
		if (jsonSources.isObject())
			for (auto const& sourceName: jsonSources.getMemberNames())
				sources[sourceName] = jsonSources[sourceName].asString();
		return compile(sources, _optimize, _readCallback);
	}
}

string compileSingle(string const& _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;
	return compile(sources, _optimize, nullptr);
}


string compileStandardInternal(string const& _input, CStyleReadFileCallback _readCallback = nullptr)
{
	StandardCompiler compiler(wrapReadCallback(_readCallback));
	return compiler.compile(_input);
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
extern char const* compileJSONCallback(char const* _input, bool _optimize, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileMulti(_input, _optimize, _readCallback);
	return s_outputBuffer.c_str();
}
extern char const* compileStandard(char const* _input, CStyleReadFileCallback _readCallback)
{
	s_outputBuffer = compileStandardInternal(_input, _readCallback);
	return s_outputBuffer.c_str();
}
}
