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
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Solidity command line interface.
 */
#include <solc/CommandLineInterface.h>

#include <solc/Exceptions.h>

#include "license.h"
#include "solidity/BuildInfo.h"

#include <libsolidity/interface/Version.h>
#include <libsolidity/ast/ASTCoqExporter.h>
#include <libsolidity/ast/ASTJsonExporter.h>
#include <libsolidity/ast/ASTJsonImporter.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/ImportRemapper.h>
#include <libsolidity/interface/StorageLayout.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/Transport.h>

#include <libyul/YulStack.h>

#include <libevmasm/Disassemble.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsmtutil/Exceptions.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/JSON.h>

#include <algorithm>
#include <fstream>
#include <memory>

#include <range/v3/view/map.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

#ifdef _WIN32 // windows
	#include <io.h>
	#define isatty _isatty
	#define fileno _fileno
#else // unix
	#include <unistd.h>
#endif

#include <fstream>

#if !defined(STDERR_FILENO)
	#define STDERR_FILENO 2
#endif

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

namespace
{

std::set<frontend::InputMode> const CompilerInputModes{
	frontend::InputMode::Compiler,
	frontend::InputMode::CompilerWithASTImport,
};

} // anonymous namespace

namespace solidity::frontend
{

std::ostream& CommandLineInterface::sout(bool _markAsUsed)
{
	if (_markAsUsed)
		m_hasOutput = true;
	return m_sout;
}

std::ostream& CommandLineInterface::serr(bool _markAsUsed)
{
	if (_markAsUsed)
		m_hasOutput = true;
	return m_serr;
}

#define cin
#define cout
#define cerr

static std::string const g_stdinFileName = "<stdin>";
static std::string const g_strAbi = "abi";
static std::string const g_strAsm = "asm";
static std::string const g_strAst = "ast";
static std::string const g_strBinary = "bin";
static std::string const g_strBinaryRuntime = "bin-runtime";
static std::string const g_strContracts = "contracts";
static std::string const g_strFunDebug = "function-debug";
static std::string const g_strFunDebugRuntime = "function-debug-runtime";
static std::string const g_strGeneratedSources = "generated-sources";
static std::string const g_strGeneratedSourcesRuntime = "generated-sources-runtime";
static std::string const g_strNatspecDev = "devdoc";
static std::string const g_strNatspecUser = "userdoc";
static std::string const g_strOpcodes = "opcodes";
static std::string const g_strSignatureHashes = "hashes";
static std::string const g_strSourceList = "sourceList";
static std::string const g_strSources = "sources";
static std::string const g_strSrcMap = "srcmap";
static std::string const g_strSrcMapRuntime = "srcmap-runtime";
static std::string const g_strStorageLayout = "storage-layout";
static std::string const g_strTransientStorageLayout = "transient-storage-layout";
static std::string const g_strVersion = "version";

static bool needsHumanTargetedStdout(CommandLineOptions const& _options)
{
	if (_options.compiler.estimateGas)
		return true;
	if (!_options.output.dir.empty())
		return false;
	return
		_options.compiler.outputs.abi ||
		_options.compiler.outputs.asm_ ||
		_options.compiler.outputs.asmJson ||
		_options.compiler.outputs.binary ||
		_options.compiler.outputs.binaryRuntime ||
		_options.compiler.outputs.metadata ||
		_options.compiler.outputs.natspecUser ||
		_options.compiler.outputs.natspecDev ||
		_options.compiler.outputs.opcodes ||
		_options.compiler.outputs.signatureHashes ||
		_options.compiler.outputs.storageLayout ||
		_options.compiler.outputs.transientStorageLayout;
}

static bool coloredOutput(CommandLineOptions const& _options)
{
	return
		(!_options.formatting.coloredOutput.has_value() && isatty(STDERR_FILENO)) ||
		(_options.formatting.coloredOutput.has_value() && _options.formatting.coloredOutput.value());
}

void CommandLineInterface::handleEVMAssembly(std::string const& _contract)
{
	solAssert(m_assemblyStack);
	solAssert(
		CompilerInputModes.count(m_options.input.mode) == 1 ||
		m_options.input.mode == frontend::InputMode::EVMAssemblerJSON
	);

	if (!m_options.compiler.outputs.asm_ && !m_options.compiler.outputs.asmJson)
		return;

	std::string assembly;
	if (m_options.compiler.outputs.asmJson)
		assembly = util::jsonPrint(m_assemblyStack->assemblyJSON(_contract), m_options.formatting.json);
	else
		assembly = m_assemblyStack->assemblyString(_contract, m_fileReader.sourceUnits());

	if (!m_options.output.dir.empty())
		createFile(
			m_compiler->filesystemFriendlyName(_contract) +
			(m_options.compiler.outputs.asmJson ? "_evm.json" : ".evm"),
			assembly
		);
	else
		sout() << "EVM assembly:" << std::endl << assembly << std::endl;
}

void CommandLineInterface::handleBinary(std::string const& _contract)
{
	solAssert(m_assemblyStack);
	solAssert(
		CompilerInputModes.count(m_options.input.mode) == 1 ||
		m_options.input.mode == frontend::InputMode::EVMAssemblerJSON
	);

	std::string binary;
	std::string binaryRuntime;
	if (m_options.compiler.outputs.binary)
		binary = objectWithLinkRefsHex(m_assemblyStack->object(_contract));
	if (m_options.compiler.outputs.binaryRuntime)
		binaryRuntime = objectWithLinkRefsHex(m_assemblyStack->runtimeObject(_contract));

	if (m_options.compiler.outputs.binary)
	{
		if (!m_options.output.dir.empty())
			createFile(m_assemblyStack->filesystemFriendlyName(_contract) + ".bin", binary);
		else
		{
			sout() << "Binary:" << std::endl;
			sout() << binary << std::endl;
		}
	}
	if (m_options.compiler.outputs.binaryRuntime)
	{
		if (!m_options.output.dir.empty())
			createFile(m_assemblyStack->filesystemFriendlyName(_contract) + ".bin-runtime", binaryRuntime);
		else
		{
			sout() << "Binary of the runtime part:" << std::endl;
			sout() << binaryRuntime << std::endl;
		}
	}
}

void CommandLineInterface::handleOpcode(std::string const& _contract)
{
	solAssert(m_assemblyStack);
	solAssert(
		CompilerInputModes.count(m_options.input.mode) == 1 ||
		m_options.input.mode == frontend::InputMode::EVMAssemblerJSON
	);

	std::string opcodes{evmasm::disassemble(m_assemblyStack->object(_contract).bytecode, m_options.output.evmVersion)};

	if (!m_options.output.dir.empty())
		createFile(m_assemblyStack->filesystemFriendlyName(_contract) + ".opcode", opcodes);
	else
	{
		sout() << "Opcodes:" << std::endl;
		sout() << std::uppercase << opcodes;
		sout() << std::endl;
	}
}

void CommandLineInterface::handleIR(std::string const& _contractName)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.ir)
		return;

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contractName) + ".yul", m_compiler->yulIR(_contractName));
	else
	{
		sout() << "IR:" << std::endl;
		sout() << m_compiler->yulIR(_contractName) << std::endl;
	}
}

void CommandLineInterface::handleIRAst(std::string const& _contractName)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.irAstJson)
		return;

	if (!m_options.output.dir.empty())
		createFile(
			m_compiler->filesystemFriendlyName(_contractName) + "_yul_ast.json",
			util::jsonPrint(
				m_compiler->yulIRAst(_contractName),
				m_options.formatting.json
			)
		);
	else
	{
		sout() << "IR AST:" << std::endl;
		sout() << util::jsonPrint(
			m_compiler->yulIRAst(_contractName),
			m_options.formatting.json
		) << std::endl;
	}
}

void CommandLineInterface::handleYulCFGExport(std::string const& _contractName)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.yulCFGJson)
		return;

	if (!m_options.output.dir.empty())
		createFile(
			m_compiler->filesystemFriendlyName(_contractName) + "_yul_cfg.json",
			util::jsonPrint(
				m_compiler->yulCFGJson(_contractName),
				m_options.formatting.json
			)
		);
	else
	{
		sout() << util::jsonPrint(
			m_compiler->yulCFGJson(_contractName),
			m_options.formatting.json
		) << std::endl;
	}
}

void CommandLineInterface::handleIROptimized(std::string const& _contractName)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.irOptimized)
		return;

	if (!m_options.output.dir.empty())
		createFile(
			m_compiler->filesystemFriendlyName(_contractName) + "_opt.yul",
			m_compiler->yulIROptimized(_contractName)
		);
	else
	{
		sout() << "Optimized IR:" << std::endl;
		sout() << m_compiler->yulIROptimized(_contractName) << std::endl;
	}
}

void CommandLineInterface::handleIROptimizedAst(std::string const& _contractName)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.irOptimizedAstJson)
		return;

	if (!m_options.output.dir.empty())
		createFile(
			m_compiler->filesystemFriendlyName(_contractName) + "_opt_yul_ast.json",
			util::jsonPrint(
				m_compiler->yulIROptimizedAst(_contractName),
				m_options.formatting.json
			)
		);
	else
	{
		sout() << "Optimized IR AST:" << std::endl;
		sout() << util::jsonPrint(
			m_compiler->yulIROptimizedAst(_contractName),
			m_options.formatting.json
		) << std::endl;
	}
}

void CommandLineInterface::handleBytecode(std::string const& _contract)
{
	solAssert(
		CompilerInputModes.count(m_options.input.mode) == 1 ||
		m_options.input.mode == frontend::InputMode::EVMAssemblerJSON
	);

	if (m_options.compiler.outputs.opcodes)
		handleOpcode(_contract);
	if (m_options.compiler.outputs.binary || m_options.compiler.outputs.binaryRuntime)
		handleBinary(_contract);
}

void CommandLineInterface::handleSignatureHashes(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.signatureHashes)
		return;

	Json interfaceSymbols = m_compiler->interfaceSymbols(_contract);
	std::string out = "Function signatures:\n";
	for (auto const& [name, value]: interfaceSymbols["methods"].items())
		out += value.get<std::string>() + ": " + name + "\n";

	if (interfaceSymbols.contains("errors"))
	{
		out += "\nError signatures:\n";
		for (auto const& [name, value]: interfaceSymbols["errors"].items())
			out += value.get<std::string>() + ": " + name + "\n";
	}

	if (interfaceSymbols.contains("events"))
	{
		out += "\nEvent signatures:\n";
		for (auto const& [name, value]: interfaceSymbols["events"].items())
			out += value.get<std::string>() + ": " + name + "\n";
	}

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".signatures", out);
	else
		sout() << out;
}

void CommandLineInterface::handleMetadata(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.metadata)
		return;

	std::string data = m_compiler->metadata(_contract);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_meta.json", data);
	else
		sout() << "Metadata:" << std::endl << data << std::endl;
}

void CommandLineInterface::handleABI(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.abi)
		return;

	std::string data = jsonPrint(removeNullMembers(m_compiler->contractABI(_contract)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".abi", data);
	else
		sout() << "Contract JSON ABI" << std::endl << data << std::endl;
}

void CommandLineInterface::handleStorageLayout(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.storageLayout)
		return;

	std::string data = jsonPrint(removeNullMembers(m_compiler->storageLayout(_contract)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_storage.json", data);
	else
		sout() << "Contract Storage Layout:" << std::endl << data << std::endl;
}

void CommandLineInterface::handleTransientStorageLayout(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.transientStorageLayout)
		return;
	std::string data = jsonPrint(removeNullMembers(m_compiler->transientStorageLayout(_contract)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_transient_storage.json", data);
	else
		sout() << "Contract Transient Storage Layout:" << std::endl << data << std::endl;
}

void CommandLineInterface::handleNatspec(bool _natspecDev, std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	bool enabled = false;
	std::string suffix;
	std::string title;

	if (_natspecDev)
	{
		enabled = m_options.compiler.outputs.natspecDev;
		suffix = ".docdev";
		title = "Developer Documentation";
	}
	else
	{
		enabled = m_options.compiler.outputs.natspecUser;
		suffix = ".docuser";
		title = "User Documentation";
	}

	if (enabled)
	{
		std::string output = jsonPrint(
			removeNullMembers(
				_natspecDev ?
				m_compiler->natspecDev(_contract) :
				m_compiler->natspecUser(_contract)
			),
			m_options.formatting.json
		);

		if (!m_options.output.dir.empty())
			createFile(m_compiler->filesystemFriendlyName(_contract) + suffix, output);
		else
		{
			sout() << title << std::endl;
			sout() << output << std::endl;
		}

	}
}

void CommandLineInterface::handleGasEstimation(std::string const& _contract)
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	Json estimates = m_compiler->gasEstimates(_contract);
	sout() << "Gas estimation:" << std::endl;

	if (estimates["creation"].is_object())
	{
		Json creation = estimates["creation"];
		sout() << "construction:" << std::endl;
		sout() << "   " << creation["executionCost"].get<std::string>();
		sout() << " + " << creation["codeDepositCost"].get<std::string>();
		sout() << " = " << creation["totalCost"].get<std::string>() << std::endl;
	}

	if (estimates["external"].is_object())
	{
		Json externalFunctions = estimates["external"];
		sout() << "external:" << std::endl;
		for (auto const& [name, value]: externalFunctions.items())
		{
			if (name.empty())
				sout() << "   fallback:\t";
			else
				sout() << "   " << name << ":\t";
			sout() << value.get<std::string>() << std::endl;
		}
	}

	if (estimates["internal"].is_object())
	{
		Json internalFunctions = estimates["internal"];
		sout() << "internal:" << std::endl;
		for (auto const& [name, value]: internalFunctions.items())
		{
			sout() << "   " << name << ":\t";
			sout() << value.get<std::string>() << std::endl;
		}
	}
}

void CommandLineInterface::readInputFiles()
{
	solAssert(!m_standardJsonInput.has_value());

	if (m_options.input.noImportCallback)
		m_universalCallback.resetImportCallback();

	static std::set<frontend::InputMode> const noInputFiles{
		frontend::InputMode::Help,
		frontend::InputMode::License,
		frontend::InputMode::Version
	};

	if (noInputFiles.count(m_options.input.mode) == 1)
		return;

	m_fileReader.setBasePath(m_options.input.basePath);

	if (m_fileReader.basePath() != "")
	{
		if (!boost::filesystem::exists(m_fileReader.basePath()))
			solThrow(CommandLineValidationError, "Base path does not exist: \"" + m_fileReader.basePath().string() + '"');

		if (!boost::filesystem::is_directory(m_fileReader.basePath()))
			solThrow(CommandLineValidationError, "Base path is not a directory: \"" + m_fileReader.basePath().string() + '"');
	}

	for (boost::filesystem::path const& includePath: m_options.input.includePaths)
		m_fileReader.addIncludePath(includePath);

	for (boost::filesystem::path const& allowedDirectory: m_options.input.allowedDirectories)
		m_fileReader.allowDirectory(allowedDirectory);

	std::map<std::string, std::set<boost::filesystem::path>> collisions =
		m_fileReader.detectSourceUnitNameCollisions(m_options.input.paths);
	if (!collisions.empty())
	{
		auto pathToQuotedString = [](boost::filesystem::path const& _path){ return "\"" + _path.string() + "\""; };

		std::string message =
			"Source unit name collision detected. "
			"The specified values of base path and/or include paths would result in multiple "
			"input files being assigned the same source unit name:\n";

		for (auto const& [sourceUnitName, normalizedInputPaths]: collisions)
		{
			message += sourceUnitName + " matches: ";
			message += util::joinHumanReadable(normalizedInputPaths | ranges::views::transform(pathToQuotedString)) + "\n";
		}

		solThrow(CommandLineValidationError, message);
	}

	for (boost::filesystem::path const& infile: m_options.input.paths)
	{
		if (!boost::filesystem::exists(infile))
		{
			if (!m_options.input.ignoreMissingFiles)
				solThrow(CommandLineValidationError, '"' + infile.string() + "\" is not found.");
			else
				report(Error::Severity::Info, fmt::format("\"{}\" is not found. Skipping.", infile.string()));

			continue;
		}

		if (!boost::filesystem::is_regular_file(infile))
		{
			if (!m_options.input.ignoreMissingFiles)
				solThrow(CommandLineValidationError, '"' + infile.string() + "\" is not a valid file.");
			else
				report(Error::Severity::Info, fmt::format("\"{}\" is not a valid file. Skipping.", infile.string()));

			continue;
		}

		// NOTE: we ignore the FileNotFound exception as we manually check above
		std::string fileContent = readFileAsString(infile);
		if (m_options.input.mode == InputMode::StandardJson)
		{
			solAssert(!m_standardJsonInput.has_value());
			m_standardJsonInput = std::move(fileContent);
		}
		else
		{
			m_fileReader.addOrUpdateFile(infile, std::move(fileContent));
			m_fileReader.allowDirectory(boost::filesystem::canonical(infile).remove_filename());
		}
	}

	if (m_options.input.addStdin)
	{
		if (m_options.input.mode == InputMode::StandardJson)
		{
			solAssert(!m_standardJsonInput.has_value());
			m_standardJsonInput = readUntilEnd(m_sin);
		}
		else
			m_fileReader.setStdin(readUntilEnd(m_sin));
	}

	if (
		m_options.input.mode != InputMode::LanguageServer &&
		m_fileReader.sourceUnits().empty() &&
		!m_standardJsonInput.has_value()
	)
		solThrow(CommandLineValidationError, "All specified input files either do not exist or are not regular files.");
}

std::map<std::string, Json> CommandLineInterface::parseAstFromInput()
{
	solAssert(m_options.input.mode == InputMode::CompilerWithASTImport);

	std::map<std::string, Json> sourceJsons;
	std::map<std::string, std::string> tmpSources;

	for (SourceCode const& sourceCode: m_fileReader.sourceUnits() | ranges::views::values)
	{
		Json ast;
		astAssert(jsonParseStrict(sourceCode, ast), "Input file could not be parsed to JSON");
		astAssert(ast.contains("sources"), "Invalid Format for import-JSON: Must have 'sources'-object");

		for (auto const& [src, value]: ast["sources"].items())
		{
			std::string astKey = value.contains("ast") ? "ast" : "AST";

			astAssert(ast["sources"][src].contains(astKey), "astkey is not member");
			astAssert(ast["sources"][src][astKey]["nodeType"].get<std::string>() == "SourceUnit",  "Top-level node should be a 'SourceUnit'");
			astAssert(sourceJsons.count(src) == 0, "All sources must have unique names");
			sourceJsons.emplace(src, std::move(value[astKey]));
			tmpSources[src] = util::jsonCompactPrint(ast);
		}
	}

	m_fileReader.setSourceUnits(tmpSources);

	return sourceJsons;
}

void CommandLineInterface::createFile(std::string const& _fileName, std::string const& _data)
{
	namespace fs = boost::filesystem;

	solAssert(!m_options.output.dir.empty());

	// NOTE: create_directories() raises an exception if the path consists solely of '.' or '..'
	// (or equivalent such as './././.'). Paths like 'a/b/.' and 'a/b/..' are fine though.
	// The simplest workaround is to use an absolute path.
	fs::create_directories(fs::absolute(m_options.output.dir));

	std::string pathName = (m_options.output.dir / _fileName).string();
	if (fs::exists(pathName) && !m_options.output.overwriteFiles)
		solThrow(CommandLineOutputError, "Refusing to overwrite existing file \"" + pathName + "\" (use --overwrite to force).");

	std::ofstream outFile(pathName);
	outFile << _data;
	if (!outFile)
		solThrow(CommandLineOutputError, "Could not write to file \"" + pathName + "\".");
}

void CommandLineInterface::createJson(std::string const& _fileName, std::string const& _json)
{
	createFile(boost::filesystem::path(_fileName).stem().string() + std::string(".json"), _json);
}

bool CommandLineInterface::run(int _argc, char const* const* _argv)
{
	try
	{
		if (!parseArguments(_argc, _argv))
			return false;

		readInputFiles();
		processInput();
		return true;
	}
	catch (CommandLineError const& _exception)
	{
		m_hasOutput = true;

		// There might be no message in the exception itself if the error output is bulky and has
		// already been printed to stderr (this happens e.g. for compiler errors).
		if (_exception.what() != ""s)
			report(Error::Severity::Error, _exception.what());

		return false;
	}
	catch (UnimplementedFeatureError const& _error)
	{
		solAssert(_error.comment(), "Unimplemented feature errors must include a message for the user");
		report(Error::Severity::Error, stringOrDefault(_error.comment(), "Unimplemented feature"));
		return false;
	}
}

bool CommandLineInterface::parseArguments(int _argc, char const* const* _argv)
{
	CommandLineParser parser;

	if (isatty(fileno(stdin)) && _argc == 1)
	{
		// If the terminal is taking input from the user, provide more user-friendly output.
		CommandLineParser::printHelp(sout());

		// In this case we want to exit with an error but not display any error message.
		return false;
	}

	try
	{
		parser.parse(_argc, _argv);
	}
	catch (...)
	{
		// Even if the overall CLI parsing fails, the --color/--no-color options may have been
		// successfully parsed, and if so, should be taken into account when printing errors.
		// If no value is present, it's possible that --no-color is still there but parsing failed
		// due to other, unrecognized options so play it safe and disable color in that case.
		m_options.formatting.coloredOutput = parser.options().formatting.coloredOutput.value_or(false);
		throw;
	}
	m_options = parser.options();

	return true;
}

void CommandLineInterface::processInput()
{
	if (m_options.output.evmVersion < EVMVersion::constantinople())
		report(
			Error::Severity::Warning,
			"Support for EVM versions older than constantinople is deprecated and will be removed in the future."
		);

	switch (m_options.input.mode)
	{
	case InputMode::Help:
		CommandLineParser::printHelp(sout());
		break;
	case InputMode::License:
		printLicense();
		break;
	case InputMode::Version:
		printVersion();
		break;
	case InputMode::StandardJson:
	{
		solAssert(m_standardJsonInput.has_value());

		StandardCompiler compiler(m_universalCallback.callback(), m_options.formatting.json);
		sout() << compiler.compile(std::move(m_standardJsonInput.value())) << std::endl;
		m_standardJsonInput.reset();
		break;
	}
	case InputMode::LanguageServer:
		serveLSP();
		break;
	case InputMode::Assembler:
		assembleYul(m_options.assembly.inputLanguage, m_options.assembly.targetMachine);
		break;
	case InputMode::Linker:
		link();
		writeLinkedFiles();
		break;
	case InputMode::Compiler:
	case InputMode::CompilerWithASTImport:
		compile();
		outputCompilationResults();
		break;
	case InputMode::EVMAssemblerJSON:
		assembleFromEVMAssemblyJSON();
		handleCombinedJSON();
		handleBytecode(m_assemblyStack->contractNames().front());
		handleEVMAssembly(m_assemblyStack->contractNames().front());
		break;
	}
}

void CommandLineInterface::printVersion()
{
	sout() << "solc, the solidity compiler commandline interface" << std::endl;
	sout() << "Version: " << solidity::frontend::VersionString << std::endl;
}

void CommandLineInterface::printLicense()
{
	sout() << otherLicenses << std::endl;
	// This is a static variable generated by cmake from LICENSE.txt
	sout() << licenseText << std::endl;
}

void CommandLineInterface::assembleFromEVMAssemblyJSON()
{
	solAssert(m_options.input.mode == InputMode::EVMAssemblerJSON);
	solAssert(!m_assemblyStack);
	solAssert(!m_evmAssemblyStack && !m_compiler);

	solAssert(m_fileReader.sourceUnits().size() == 1);
	auto&& [sourceUnitName, source] = *m_fileReader.sourceUnits().begin();

	auto evmAssemblyStack = std::make_unique<evmasm::EVMAssemblyStack>(m_options.output.evmVersion);
	try
	{
		evmAssemblyStack->parseAndAnalyze(sourceUnitName, source);
	}
	catch (evmasm::AssemblyImportException const& _exception)
	{
		solThrow(CommandLineExecutionError, "Assembly Import Error: "s + _exception.what());
	}

	if (m_options.output.debugInfoSelection.has_value())
		evmAssemblyStack->selectDebugInfo(m_options.output.debugInfoSelection.value());
	evmAssemblyStack->assemble();

	m_evmAssemblyStack = std::move(evmAssemblyStack);
	m_assemblyStack = m_evmAssemblyStack.get();
}

void CommandLineInterface::compile()
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);
	solAssert(!m_assemblyStack);
	solAssert(!m_evmAssemblyStack && !m_compiler);

	m_compiler = std::make_unique<CompilerStack>(m_universalCallback.callback());
	m_assemblyStack = m_compiler.get();

	SourceReferenceFormatter formatter(serr(false), *m_compiler, coloredOutput(m_options), m_options.formatting.withErrorIds);

	try
	{
		if (m_options.metadata.literalSources)
			m_compiler->useMetadataLiteralSources(true);
		m_compiler->setMetadataFormat(m_options.metadata.format);
		m_compiler->setMetadataHash(m_options.metadata.hash);
		if (m_options.modelChecker.initialize)
			m_compiler->setModelCheckerSettings(m_options.modelChecker.settings);
		m_compiler->setRemappings(m_options.input.remappings);
		m_compiler->setLibraries(m_options.linker.libraries);
		m_compiler->setViaIR(m_options.output.viaIR);
		m_compiler->setEVMVersion(m_options.output.evmVersion);
		m_compiler->setEOFVersion(m_options.output.eofVersion);
		m_compiler->setRevertStringBehaviour(m_options.output.revertStrings);
		if (m_options.output.debugInfoSelection.has_value())
			m_compiler->selectDebugInfo(m_options.output.debugInfoSelection.value());

		CompilerStack::IROutputSelection irOutputSelection = CompilerStack::IROutputSelection::None;
		if (m_options.compiler.outputs.irOptimized || m_options.compiler.outputs.irOptimizedAstJson || m_options.compiler.outputs.yulCFGJson)
			irOutputSelection = CompilerStack::IROutputSelection::UnoptimizedAndOptimized;
		else if (m_options.compiler.outputs.ir || m_options.compiler.outputs.irAstJson)
			irOutputSelection = CompilerStack::IROutputSelection::UnoptimizedOnly;

		m_compiler->requestIROutputs(irOutputSelection);
		m_compiler->enableEvmBytecodeGeneration(
			m_options.compiler.estimateGas ||
			m_options.compiler.outputs.asm_ ||
			m_options.compiler.outputs.asmJson ||
			m_options.compiler.outputs.opcodes ||
			m_options.compiler.outputs.binary ||
			m_options.compiler.outputs.binaryRuntime ||
			(m_options.compiler.combinedJsonRequests && (
				m_options.compiler.combinedJsonRequests->binary ||
				m_options.compiler.combinedJsonRequests->binaryRuntime ||
				m_options.compiler.combinedJsonRequests->opcodes ||
				m_options.compiler.combinedJsonRequests->asm_ ||
				m_options.compiler.combinedJsonRequests->generatedSources ||
				m_options.compiler.combinedJsonRequests->generatedSourcesRuntime ||
				m_options.compiler.combinedJsonRequests->srcMap ||
				m_options.compiler.combinedJsonRequests->srcMapRuntime ||
				m_options.compiler.combinedJsonRequests->funDebug ||
				m_options.compiler.combinedJsonRequests->funDebugRuntime
			))
		);

		m_compiler->setOptimiserSettings(m_options.optimiserSettings());

		if (m_options.input.mode == InputMode::CompilerWithASTImport)
		{
			try
			{
				m_compiler->importASTs(parseAstFromInput());

				if (!m_compiler->analyze())
				{
					formatter.printErrorInformation(m_compiler->errors());
					astAssert(false, "Analysis of the AST failed");
				}
			}
			catch (Exception const& _exc)
			{
				// FIXME: AST import is missing proper validations. This hack catches failing
				// assertions and presents them as if they were compiler errors.
				solThrow(CommandLineExecutionError, "Failed to import AST: "s + _exc.what());
			}
		}
		else
			m_compiler->setSources(m_fileReader.sourceUnits());

		bool successful = m_compiler->compile(m_options.output.stopAfter);

		for (auto const& error: m_compiler->errors())
		{
			m_hasOutput = true;
			formatter.printErrorInformation(*error);
		}

		if (!successful)
			solThrow(CommandLineExecutionError, "");
	}
	catch (CompilerError const& _exception)
	{
		m_hasOutput = true;
		formatter.printExceptionInformation(
			_exception,
			Error::errorSeverity(Error::Type::CompilerError)
		);
		solThrow(CommandLineExecutionError, "");
	}
}

void CommandLineInterface::handleCombinedJSON()
{
	solAssert(m_assemblyStack);
	solAssert(
		CompilerInputModes.count(m_options.input.mode) == 1 ||
		m_options.input.mode == frontend::InputMode::EVMAssemblerJSON
	);

	if (!m_options.compiler.combinedJsonRequests.has_value())
		return;

	Json output;

	output[g_strVersion] = frontend::VersionString;
	std::vector<std::string> contracts = m_assemblyStack->contractNames();

	if (!contracts.empty())
		output[g_strContracts] = Json::object();
	for (std::string const& contractName: contracts)
	{
		Json& contractData = output[g_strContracts][contractName] = Json::object();

		// NOTE: The state checks here are more strict that in Standard JSON. There we allow
		// requesting certain outputs even if compilation fails as long as analysis went ok.
		if (m_compiler && m_compiler->compilationSuccessful())
		{
			if (m_options.compiler.combinedJsonRequests->abi)
				contractData[g_strAbi] = m_compiler->contractABI(contractName);
			if (m_options.compiler.combinedJsonRequests->metadata)
				contractData["metadata"] = m_compiler->metadata(contractName);
			if (m_options.compiler.combinedJsonRequests->storageLayout)
				contractData[g_strStorageLayout] = m_compiler->storageLayout(contractName);
			if (m_options.compiler.combinedJsonRequests->transientStorageLayout)
				contractData[g_strTransientStorageLayout] = m_compiler->transientStorageLayout(contractName);
			if (m_options.compiler.combinedJsonRequests->generatedSources)
				contractData[g_strGeneratedSources] = m_compiler->generatedSources(contractName, false);
			if (m_options.compiler.combinedJsonRequests->generatedSourcesRuntime)
				contractData[g_strGeneratedSourcesRuntime] = m_compiler->generatedSources(contractName, true);
			if (m_options.compiler.combinedJsonRequests->signatureHashes)
				contractData[g_strSignatureHashes] = m_compiler->interfaceSymbols(contractName)["methods"];
			if (m_options.compiler.combinedJsonRequests->natspecDev)
				contractData[g_strNatspecDev] = m_compiler->natspecDev(contractName);
			if (m_options.compiler.combinedJsonRequests->natspecUser)
				contractData[g_strNatspecUser] = m_compiler->natspecUser(contractName);
		}

		if (m_assemblyStack->compilationSuccessful())
		{
			if (m_options.compiler.combinedJsonRequests->binary)
				contractData[g_strBinary] = m_assemblyStack->object(contractName).toHex();
			if (m_options.compiler.combinedJsonRequests->binaryRuntime)
				contractData[g_strBinaryRuntime] = m_assemblyStack->runtimeObject(contractName).toHex();
			if (m_options.compiler.combinedJsonRequests->opcodes)
				contractData[g_strOpcodes] = evmasm::disassemble(m_assemblyStack->object(contractName).bytecode, m_options.output.evmVersion);
			if (m_options.compiler.combinedJsonRequests->asm_)
				contractData[g_strAsm] = m_assemblyStack->assemblyJSON(contractName);
			if (m_options.compiler.combinedJsonRequests->srcMap)
			{
				auto map = m_assemblyStack->sourceMapping(contractName);
				contractData[g_strSrcMap] = map ? *map : "";
			}
			if (m_options.compiler.combinedJsonRequests->srcMapRuntime)
			{
				auto map = m_assemblyStack->runtimeSourceMapping(contractName);
				contractData[g_strSrcMapRuntime] = map ? *map : "";
			}
			if (m_options.compiler.combinedJsonRequests->funDebug)
				contractData[g_strFunDebug] = StandardCompiler::formatFunctionDebugData(
					m_assemblyStack->object(contractName).functionDebugData
				);
			if (m_options.compiler.combinedJsonRequests->funDebugRuntime)
				contractData[g_strFunDebugRuntime] = StandardCompiler::formatFunctionDebugData(
					m_assemblyStack->runtimeObject(contractName).functionDebugData
				);
		}
	}

	bool needsSourceList =
		m_options.compiler.combinedJsonRequests->ast ||
		m_options.compiler.combinedJsonRequests->srcMap ||
		m_options.compiler.combinedJsonRequests->srcMapRuntime;
	if (needsSourceList)
	{
		// Indices into this array are used to abbreviate source names in source locations.
		output[g_strSourceList] = Json::array();

		for (auto const& source: m_assemblyStack->sourceNames())
			output[g_strSourceList].emplace_back(source);
	}

	if (m_options.compiler.combinedJsonRequests->ast)
	{
		solAssert(m_compiler);
		output[g_strSources] = Json::object();
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			output[g_strSources][sourceCode.first] = Json::object();
			output[g_strSources][sourceCode.first]["AST"] = ASTJsonExporter(
				m_compiler->state(),
				m_compiler->sourceIndices()
			).toJson(m_compiler->ast(sourceCode.first));
			output[g_strSources][sourceCode.first]["id"] = m_compiler->sourceIndices().at(sourceCode.first);
		}
	}

	std::string json = jsonPrint(removeNullMembers(std::move(output)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createJson("combined", json);
	else
		sout() << json << std::endl;
}

void CommandLineInterface::handleAst()
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	if (!m_options.compiler.outputs.astCompactJson)
		return;

	std::vector<ASTNode const*> asts;
	for (auto const& sourceCode: m_fileReader.sourceUnits())
		asts.push_back(&m_compiler->ast(sourceCode.first));

	if (!m_options.output.dir.empty())
	{
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			std::stringstream data;
			std::string postfix = "";
			ASTJsonExporter(m_compiler->state(), m_compiler->sourceIndices()).print(data, m_compiler->ast(sourceCode.first), m_options.formatting.json);
			postfix += "_json";
			boost::filesystem::path path(sourceCode.first);
			createFile(path.filename().string() + postfix + ".ast", data.str());
		}
	}
	else
	{
		sout() << "JSON AST (compact format):" << std::endl << std::endl;
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			sout() << std::endl << "======= " << sourceCode.first << " =======" << std::endl;
			ASTJsonExporter(m_compiler->state(), m_compiler->sourceIndices()).print(sout(), m_compiler->ast(sourceCode.first), m_options.formatting.json);
		}
	}
}

void CommandLineInterface::handleCoq()
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	// if (!m_options.compiler.outputs.astCompactJson)
	// 	return;

	std::vector<ASTNode const*> asts;
	for (auto const& sourceCode: m_fileReader.sourceUnits())
		asts.push_back(&m_compiler->ast(sourceCode.first));

	if (!m_options.output.dir.empty())
	{
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			std::stringstream data;
			std::string postfix = "";
			ASTCoqExporter(m_compiler->state(), m_compiler->sourceIndices()).print(data, m_compiler->ast(sourceCode.first));
			boost::filesystem::path path(sourceCode.first);
			path.replace_extension(".v");
			createFile(path.filename().string(), data.str());
		}
	}
	else
	{
		sout() << "Coq AST:" << std::endl << std::endl;
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			sout() << std::endl << "======= " << sourceCode.first << " =======" << std::endl;
			ASTCoqExporter(m_compiler->state(), m_compiler->sourceIndices()).print(sout(), m_compiler->ast(sourceCode.first));
		}
	}
}

void CommandLineInterface::serveLSP()
{
	lsp::StdioTransport transport;
	if (!lsp::LanguageServer{transport}.run())
		solThrow(CommandLineExecutionError, "LSP terminated abnormally.");
}

void CommandLineInterface::link()
{
	solAssert(m_options.input.mode == InputMode::Linker);

	// Map from how the libraries will be named inside the bytecode to their addresses.
	std::map<std::string, h160> librariesReplacements;
	int const placeholderSize = 40; // 20 bytes or 40 hex characters
	for (auto const& library: m_options.linker.libraries)
	{
		std::string const& name = library.first;
		// Library placeholders are 40 hex digits (20 bytes) that start and end with '__'.
		// This leaves 36 characters for the library identifier. The identifier used to
		// be just the cropped or '_'-padded library name, but this changed to
		// the cropped hex representation of the hash of the library name.
		// We support both ways of linking here.
		librariesReplacements["__" + evmasm::LinkerObject::libraryPlaceholder(name) + "__"] = library.second;

		std::string replacement = "__";
		for (size_t i = 0; i < placeholderSize - 4; ++i)
			replacement.push_back(i < name.size() ? name[i] : '_');
		replacement += "__";
		librariesReplacements[replacement] = library.second;
	}

	FileReader::StringMap sourceCodes = m_fileReader.sourceUnits();
	for (auto& src: sourceCodes)
	{
		auto end = src.second.end();
		for (auto it = src.second.begin(); it != end;)
		{
			while (it != end && *it != '_') ++it;
			if (it == end) break;
			if (
				end - it < placeholderSize ||
				*(it + 1) != '_' ||
				*(it + placeholderSize - 2) != '_' ||
				*(it + placeholderSize - 1) != '_'
			)
				solThrow(
					CommandLineExecutionError,
					"Error in binary object file " + src.first + " at position " + std::to_string(it - src.second.begin()) + "\n" +
					'"' + std::string(it, it + std::min(placeholderSize, static_cast<int>(end - it))) + "\" is not a valid link reference."
				);

			std::string foundPlaceholder(it, it + placeholderSize);
			if (librariesReplacements.count(foundPlaceholder))
			{
				std::string hexStr(util::toHex(librariesReplacements.at(foundPlaceholder).asBytes()));
				copy(hexStr.begin(), hexStr.end(), it);
			}
			else
				report(
					Error::Severity::Warning,
					fmt::format("Reference \"{}\" in file \"{}\" still unresolved.", foundPlaceholder, src.first)
				);
			it += placeholderSize;
		}
		// Remove hints for resolved libraries.
		for (auto const& library: m_options.linker.libraries)
			boost::algorithm::erase_all(src.second, "\n" + libraryPlaceholderHint(library.first));
		while (!src.second.empty() && *prev(src.second.end()) == '\n')
			src.second.resize(src.second.size() - 1);
	}
	m_fileReader.setSourceUnits(std::move(sourceCodes));
}

void CommandLineInterface::writeLinkedFiles()
{
	solAssert(m_options.input.mode == InputMode::Linker);

	for (auto const& src: m_fileReader.sourceUnits())
		if (src.first == g_stdinFileName)
			sout() << src.second << std::endl;
		else
		{
			std::ofstream outFile(src.first);
			outFile << src.second;
			if (!outFile)
				solThrow(CommandLineOutputError, "Could not write to file " + src.first + ". Aborting.");
		}
	sout() << "Linking completed." << std::endl;
}

std::string CommandLineInterface::libraryPlaceholderHint(std::string const& _libraryName)
{
	return "// " + evmasm::LinkerObject::libraryPlaceholder(_libraryName) + " -> " + _libraryName;
}

std::string CommandLineInterface::objectWithLinkRefsHex(evmasm::LinkerObject const& _obj)
{
	std::string out = _obj.toHex();
	if (!_obj.linkReferences.empty())
	{
		out += "\n";
		for (auto const& linkRef: _obj.linkReferences)
			out += "\n" + libraryPlaceholderHint(linkRef.second);
	}
	return out;
}

void CommandLineInterface::assembleYul(yul::YulStack::Language _language, yul::YulStack::Machine _targetMachine)
{
	solAssert(m_options.input.mode == InputMode::Assembler);

	bool successful = true;
	std::map<std::string, yul::YulStack> yulStacks;
	for (auto const& src: m_fileReader.sourceUnits())
	{
		auto& stack = yulStacks[src.first] = yul::YulStack(
			m_options.output.evmVersion,
			m_options.output.eofVersion,
			_language,
			m_options.optimiserSettings(),
			m_options.output.debugInfoSelection.has_value() ?
				m_options.output.debugInfoSelection.value() :
				DebugInfoSelection::Default()
		);

		if (!stack.parseAndAnalyze(src.first, src.second))
			successful = false;
		else
			stack.optimize();

		if (successful && m_options.compiler.outputs.asmJson)
		{
			std::shared_ptr<yul::Object> result = stack.parserResult();
			if (result && !result->hasContiguousSourceIndices())
				solThrow(
					CommandLineExecutionError,
					"Generating the assembly JSON output was not possible. "
					"Source indices provided in the @use-src annotation in the Yul input do not start at 0 or are not contiguous."
				);
		}
	}

	for (auto const& sourceAndStack: yulStacks)
	{
		auto const& stack = sourceAndStack.second;
		SourceReferenceFormatter formatter(serr(false), stack, coloredOutput(m_options), m_options.formatting.withErrorIds);

		for (auto const& error: stack.errors())
		{
			m_hasOutput = true;
			formatter.printErrorInformation(*error);
		}
		if (Error::containsErrors(stack.errors()))
			successful = false;
	}

	if (!successful)
	{
		solAssert(m_hasOutput);
		solThrow(CommandLineExecutionError, "");
	}

	for (auto const& src: m_fileReader.sourceUnits())
	{
		solAssert(_targetMachine == yul::YulStack::Machine::EVM);
		std::string machine = "EVM";
		sout() << std::endl << "======= " << src.first << " (" << machine << ") =======" << std::endl;

		yul::YulStack& stack = yulStacks[src.first];

		if (m_options.compiler.outputs.irOptimized)
		{
			// NOTE: This actually outputs unoptimized code when the optimizer is disabled but
			// 'ir' output in StandardCompiler works the same way.
			sout() << std::endl << "Pretty printed source:" << std::endl;
			sout() << stack.print() << std::endl;
		}

		yul::MachineAssemblyObject object;
		object = stack.assemble(_targetMachine);
		object.bytecode->link(m_options.linker.libraries);

		if (m_options.compiler.outputs.binary)
		{
			sout() << std::endl << "Binary representation:" << std::endl;
			if (object.bytecode)
				sout() << object.bytecode->toHex() << std::endl;
			else
				report(Error::Severity::Info, "No binary representation found.");
		}
		if (m_options.compiler.outputs.astCompactJson)
		{
			sout() << "AST:" << std::endl << std::endl;
			sout() << util::jsonPrint(stack.astJson(), m_options.formatting.json) << std::endl;
		}
		if (m_options.compiler.outputs.yulCFGJson)
		{
			sout() << "Yul Control Flow Graph:" << std::endl << std::endl;
			sout() << util::jsonPrint(stack.cfgJson(), m_options.formatting.json) << std::endl;
		}
		solAssert(_targetMachine == yul::YulStack::Machine::EVM, "");
		if (m_options.compiler.outputs.asm_)
		{
			sout() << std::endl << "Text representation:" << std::endl;
			std::string assemblyText{object.assembly->assemblyString(stack.debugInfoSelection())};
			if (!assemblyText.empty())
				sout() << assemblyText << std::endl;
			else
				report(Error::Severity::Info, "No text representation found.");
		}
		if (m_options.compiler.outputs.asmJson)
		{
			sout() << std::endl << "EVM assembly:" << std::endl;
			std::map<std::string, unsigned> sourceIndices;
			stack.parserResult()->collectSourceIndices(sourceIndices);
			sout() << util::jsonPrint(
				object.assembly->assemblyJSON(sourceIndices),
				m_options.formatting.json
			) << std::endl;
		}
	}
}

void CommandLineInterface::outputCompilationResults()
{
	solAssert(CompilerInputModes.count(m_options.input.mode) == 1);

	handleCombinedJSON();

	// do we need AST output?
	handleAst();

	handleCoq();

	CompilerOutputs astOutputSelection;
	astOutputSelection.astCompactJson = true;
	if (m_options.compiler.outputs != CompilerOutputs() && m_options.compiler.outputs != astOutputSelection)
	{
		// Currently AST is the only output allowed with --stop-after parsing. For all of the others
		// we can safely assume that full compilation was performed and successful.
		solAssert(m_options.output.stopAfter >= CompilerStack::State::CompilationSuccessful);

		for (std::string const& contract: m_compiler->contractNames())
		{
			if (needsHumanTargetedStdout(m_options))
				sout() << std::endl << "======= " << contract << " =======" << std::endl;

			handleEVMAssembly(contract);

			if (m_options.compiler.estimateGas)
				handleGasEstimation(contract);

			handleBytecode(contract);
			handleIR(contract);
			handleIRAst(contract);
			handleIROptimized(contract);
			handleIROptimizedAst(contract);
			handleYulCFGExport(contract);
			handleSignatureHashes(contract);
			handleMetadata(contract);
			handleABI(contract);
			handleStorageLayout(contract);
			handleTransientStorageLayout(contract);
			handleNatspec(true, contract);
			handleNatspec(false, contract);
		} // end of contracts iteration
	}

	if (!m_hasOutput)
	{
		if (!m_options.output.dir.empty())
			sout() << "Compiler run successful. Artifact(s) can be found in directory " << m_options.output.dir << "." << std::endl;
		else if (m_compiler->contractNames().empty())
			sout() << "Compiler run successful. No contracts to compile." << std::endl;
		else
			sout() << "Compiler run successful. No output generated." << std::endl;
	}
}

void CommandLineInterface::report(langutil::Error::Severity _severity, std::string _message)
{
	SourceReferenceFormatter::printPrimaryMessage(
		serr(),
		_message,
		_severity,
		std::nullopt,
		coloredOutput(m_options),
		m_options.formatting.withErrorIds
	);
}

}
