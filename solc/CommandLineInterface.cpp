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

#include <libevmasm/Instruction.h>
#include <libevmasm/Disassemble.h>
#include <libevmasm/GasMeter.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/Scanner.h>
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
#include <boost/range/adaptor/filtered.hpp>
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


using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

namespace solidity::frontend
{

ostream& CommandLineInterface::sout(bool _markAsUsed)
{
	if (_markAsUsed)
		m_hasOutput = true;
	return m_sout;
}

ostream& CommandLineInterface::serr(bool _markAsUsed)
{
	if (_markAsUsed)
		m_hasOutput = true;
	return m_serr;
}

#define cin
#define cout
#define cerr

static string const g_stdinFileName = "<stdin>";
static string const g_strAbi = "abi";
static string const g_strAsm = "asm";
static string const g_strAst = "ast";
static string const g_strBinary = "bin";
static string const g_strBinaryRuntime = "bin-runtime";
static string const g_strContracts = "contracts";
static string const g_strFunDebug = "function-debug";
static string const g_strFunDebugRuntime = "function-debug-runtime";
static string const g_strGeneratedSources = "generated-sources";
static string const g_strGeneratedSourcesRuntime = "generated-sources-runtime";
static string const g_strNatspecDev = "devdoc";
static string const g_strNatspecUser = "userdoc";
static string const g_strOpcodes = "opcodes";
static string const g_strSignatureHashes = "hashes";
static string const g_strSourceList = "sourceList";
static string const g_strSources = "sources";
static string const g_strSrcMap = "srcmap";
static string const g_strSrcMapRuntime = "srcmap-runtime";
static string const g_strStorageLayout = "storage-layout";
static string const g_strVersion = "version";

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
		_options.compiler.outputs.storageLayout;
}

static bool coloredOutput(CommandLineOptions const& _options)
{
	return
		(!_options.formatting.coloredOutput.has_value() && isatty(STDERR_FILENO)) ||
		(_options.formatting.coloredOutput.has_value() && _options.formatting.coloredOutput.value());
}

void CommandLineInterface::handleBinary(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (m_options.compiler.outputs.binary)
	{
		if (!m_options.output.dir.empty())
			createFile(m_compiler->filesystemFriendlyName(_contract) + ".bin", objectWithLinkRefsHex(m_compiler->object(_contract)));
		else
		{
			sout() << "Binary:" << endl;
			sout() << objectWithLinkRefsHex(m_compiler->object(_contract)) << endl;
		}
	}
	if (m_options.compiler.outputs.binaryRuntime)
	{
		if (!m_options.output.dir.empty())
			createFile(m_compiler->filesystemFriendlyName(_contract) + ".bin-runtime", objectWithLinkRefsHex(m_compiler->runtimeObject(_contract)));
		else
		{
			sout() << "Binary of the runtime part:" << endl;
			sout() << objectWithLinkRefsHex(m_compiler->runtimeObject(_contract)) << endl;
		}
	}
}

void CommandLineInterface::handleOpcode(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".opcode", evmasm::disassemble(m_compiler->object(_contract).bytecode));
	else
	{
		sout() << "Opcodes:" << endl;
		sout() << std::uppercase << evmasm::disassemble(m_compiler->object(_contract).bytecode);
		sout() << endl;
	}
}

void CommandLineInterface::handleIR(string const& _contractName)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.ir)
		return;

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contractName) + ".yul", m_compiler->yulIR(_contractName));
	else
	{
		sout() << "IR:" << endl;
		sout() << m_compiler->yulIR(_contractName) << endl;
	}
}

void CommandLineInterface::handleIROptimized(string const& _contractName)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.irOptimized)
		return;

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contractName) + "_opt.yul", m_compiler->yulIROptimized(_contractName));
	else
	{
		sout() << "Optimized IR:" << endl;
		sout() << m_compiler->yulIROptimized(_contractName) << endl;
	}
}

void CommandLineInterface::handleEwasm(string const& _contractName)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.ewasm)
		return;

	if (!m_options.output.dir.empty())
	{
		createFile(m_compiler->filesystemFriendlyName(_contractName) + ".wast", m_compiler->ewasm(_contractName));
		createFile(
			m_compiler->filesystemFriendlyName(_contractName) + ".wasm",
			asString(m_compiler->ewasmObject(_contractName).bytecode)
		);
	}
	else
	{
		sout() << "Ewasm text:" << endl;
		sout() << m_compiler->ewasm(_contractName) << endl;
		sout() << "Ewasm binary (hex): " << m_compiler->ewasmObject(_contractName).toHex() << endl;
	}
}

void CommandLineInterface::handleBytecode(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (m_options.compiler.outputs.opcodes)
		handleOpcode(_contract);
	if (m_options.compiler.outputs.binary || m_options.compiler.outputs.binaryRuntime)
		handleBinary(_contract);
}

void CommandLineInterface::handleSignatureHashes(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.signatureHashes)
		return;

	Json::Value interfaceSymbols = m_compiler->interfaceSymbols(_contract);
	string out = "Function signatures:\n";
	for (auto const& name: interfaceSymbols["methods"].getMemberNames())
		out += interfaceSymbols["methods"][name].asString() + ": " + name + "\n";

	if (interfaceSymbols.isMember("errors"))
	{
		out += "\nError signatures:\n";
		for (auto const& name: interfaceSymbols["errors"].getMemberNames())
			out += interfaceSymbols["errors"][name].asString() + ": " + name + "\n";
	}

	if (interfaceSymbols.isMember("events"))
	{
		out += "\nEvent signatures:\n";
		for (auto const& name: interfaceSymbols["events"].getMemberNames())
			out += interfaceSymbols["events"][name].asString() + ": " + name + "\n";
	}

	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".signatures", out);
	else
		sout() << out;
}

void CommandLineInterface::handleMetadata(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.metadata)
		return;

	string data = m_compiler->metadata(_contract);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_meta.json", data);
	else
		sout() << "Metadata:" << endl << data << endl;
}

void CommandLineInterface::handleABI(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.abi)
		return;

	string data = jsonPrint(removeNullMembers(m_compiler->contractABI(_contract)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + ".abi", data);
	else
		sout() << "Contract JSON ABI" << endl << data << endl;
}

void CommandLineInterface::handleStorageLayout(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.storageLayout)
		return;

	string data = jsonPrint(removeNullMembers(m_compiler->storageLayout(_contract)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createFile(m_compiler->filesystemFriendlyName(_contract) + "_storage.json", data);
	else
		sout() << "Contract Storage Layout:" << endl << data << endl;
}

void CommandLineInterface::handleNatspec(bool _natspecDev, string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

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
			sout() << title << endl;
			sout() << output << endl;
		}

	}
}

void CommandLineInterface::handleGasEstimation(string const& _contract)
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	Json::Value estimates = m_compiler->gasEstimates(_contract);
	sout() << "Gas estimation:" << endl;

	if (estimates["creation"].isObject())
	{
		Json::Value creation = estimates["creation"];
		sout() << "construction:" << endl;
		sout() << "   " << creation["executionCost"].asString();
		sout() << " + " << creation["codeDepositCost"].asString();
		sout() << " = " << creation["totalCost"].asString() << endl;
	}

	if (estimates["external"].isObject())
	{
		Json::Value externalFunctions = estimates["external"];
		sout() << "external:" << endl;
		for (auto const& name: externalFunctions.getMemberNames())
		{
			if (name.empty())
				sout() << "   fallback:\t";
			else
				sout() << "   " << name << ":\t";
			sout() << externalFunctions[name].asString() << endl;
		}
	}

	if (estimates["internal"].isObject())
	{
		Json::Value internalFunctions = estimates["internal"];
		sout() << "internal:" << endl;
		for (auto const& name: internalFunctions.getMemberNames())
		{
			sout() << "   " << name << ":\t";
			sout() << internalFunctions[name].asString() << endl;
		}
	}
}

void CommandLineInterface::readInputFiles()
{
	solAssert(!m_standardJsonInput.has_value(), "");

	if (
		m_options.input.mode == InputMode::Help ||
		m_options.input.mode == InputMode::License ||
		m_options.input.mode == InputMode::Version
	)
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

	map<std::string, set<boost::filesystem::path>> collisions =
		m_fileReader.detectSourceUnitNameCollisions(m_options.input.paths);
	if (!collisions.empty())
	{
		auto pathToQuotedString = [](boost::filesystem::path const& _path){ return "\"" + _path.string() + "\""; };

		string message =
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
				serr() << infile << " is not found. Skipping." << endl;

			continue;
		}

		if (!boost::filesystem::is_regular_file(infile))
		{
			if (!m_options.input.ignoreMissingFiles)
				solThrow(CommandLineValidationError, '"' + infile.string() + "\" is not a valid file.");
			else
				serr() << infile << " is not a valid file. Skipping." << endl;

			continue;
		}

		// NOTE: we ignore the FileNotFound exception as we manually check above
		string fileContent = readFileAsString(infile);
		if (m_options.input.mode == InputMode::StandardJson)
		{
			solAssert(!m_standardJsonInput.has_value(), "");
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
			solAssert(!m_standardJsonInput.has_value(), "");
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

map<string, Json::Value> CommandLineInterface::parseAstFromInput()
{
	solAssert(m_options.input.mode == InputMode::CompilerWithASTImport, "");

	map<string, Json::Value> sourceJsons;
	map<string, string> tmpSources;

	for (SourceCode const& sourceCode: m_fileReader.sourceUnits() | ranges::views::values)
	{
		Json::Value ast;
		astAssert(jsonParseStrict(sourceCode, ast), "Input file could not be parsed to JSON");
		astAssert(ast.isMember("sources"), "Invalid Format for import-JSON: Must have 'sources'-object");

		for (auto& src: ast["sources"].getMemberNames())
		{
			std::string astKey = ast["sources"][src].isMember("ast") ? "ast" : "AST";

			astAssert(ast["sources"][src].isMember(astKey), "astkey is not member");
			astAssert(ast["sources"][src][astKey]["nodeType"].asString() == "SourceUnit",  "Top-level node should be a 'SourceUnit'");
			astAssert(sourceJsons.count(src) == 0, "All sources must have unique names");
			sourceJsons.emplace(src, std::move(ast["sources"][src][astKey]));
			tmpSources[src] = util::jsonCompactPrint(ast);
		}
	}

	m_fileReader.setSourceUnits(tmpSources);

	return sourceJsons;
}

void CommandLineInterface::createFile(string const& _fileName, string const& _data)
{
	namespace fs = boost::filesystem;

	solAssert(!m_options.output.dir.empty(), "");

	// NOTE: create_directories() raises an exception if the path consists solely of '.' or '..'
	// (or equivalent such as './././.'). Paths like 'a/b/.' and 'a/b/..' are fine though.
	// The simplest workaround is to use an absolute path.
	fs::create_directories(fs::absolute(m_options.output.dir));

	string pathName = (m_options.output.dir / _fileName).string();
	if (fs::exists(pathName) && !m_options.output.overwriteFiles)
		solThrow(CommandLineOutputError, "Refusing to overwrite existing file \"" + pathName + "\" (use --overwrite to force).");

	ofstream outFile(pathName);
	outFile << _data;
	if (!outFile)
		solThrow(CommandLineOutputError, "Could not write to file \"" + pathName + "\".");
}

void CommandLineInterface::createJson(string const& _fileName, string const& _json)
{
	createFile(boost::filesystem::basename(_fileName) + string(".json"), _json);
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
			serr() << _exception.what() << endl;

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

	parser.parse(_argc, _argv);
	m_options = parser.options();

	return true;
}

void CommandLineInterface::processInput()
{
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
		solAssert(m_standardJsonInput.has_value(), "");

		StandardCompiler compiler(m_fileReader.reader(), m_options.formatting.json);
		sout() << compiler.compile(std::move(m_standardJsonInput.value())) << endl;
		m_standardJsonInput.reset();
		break;
	}
	case InputMode::LanguageServer:
		serveLSP();
		break;
	case InputMode::Assembler:
		assemble(m_options.assembly.inputLanguage, m_options.assembly.targetMachine);
		break;
	case InputMode::Linker:
		link();
		writeLinkedFiles();
		break;
	case InputMode::Compiler:
	case InputMode::CompilerWithASTImport:
		compile();
		outputCompilationResults();
	}
}

void CommandLineInterface::printVersion()
{
	sout() << "solc, the solidity compiler commandline interface" << endl;
	sout() << "Version: " << solidity::frontend::VersionString << endl;
}

void CommandLineInterface::printLicense()
{
	sout() << otherLicenses << endl;
	// This is a static variable generated by cmake from LICENSE.txt
	sout() << licenseText << endl;
}

void CommandLineInterface::compile()
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	m_compiler = make_unique<CompilerStack>(m_fileReader.reader());

	SourceReferenceFormatter formatter(serr(false), *m_compiler, coloredOutput(m_options), m_options.formatting.withErrorIds);

	try
	{
		if (m_options.metadata.literalSources)
			m_compiler->useMetadataLiteralSources(true);
		m_compiler->setMetadataHash(m_options.metadata.hash);
		if (m_options.modelChecker.initialize)
			m_compiler->setModelCheckerSettings(m_options.modelChecker.settings);
		m_compiler->setRemappings(m_options.input.remappings);
		m_compiler->setLibraries(m_options.linker.libraries);
		m_compiler->setViaIR(m_options.output.viaIR);
		m_compiler->setEVMVersion(m_options.output.evmVersion);
		m_compiler->setRevertStringBehaviour(m_options.output.revertStrings);
		if (m_options.output.debugInfoSelection.has_value())
			m_compiler->selectDebugInfo(m_options.output.debugInfoSelection.value());
		// TODO: Perhaps we should not compile unless requested

		m_compiler->enableIRGeneration(m_options.compiler.outputs.ir || m_options.compiler.outputs.irOptimized);
		m_compiler->enableEwasmGeneration(m_options.compiler.outputs.ewasm);
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
		{
			m_compiler->setSources(m_fileReader.sourceUnits());
			m_compiler->setParserErrorRecovery(m_options.input.errorRecovery);
		}

		bool successful = m_compiler->compile(m_options.output.stopAfter);

		for (auto const& error: m_compiler->errors())
		{
			m_hasOutput = true;
			formatter.printErrorInformation(*error);
		}

		if (!successful && !m_options.input.errorRecovery)
			solThrow(CommandLineExecutionError, "");
	}
	catch (CompilerError const& _exception)
	{
		m_hasOutput = true;
		formatter.printExceptionInformation(_exception, Error::Type::CompilerError);
		solThrow(CommandLineExecutionError, "");
	}
	catch (Error const& _error)
	{
		if (_error.type() == Error::Type::DocstringParsingError)
		{
			serr() << *boost::get_error_info<errinfo_comment>(_error);
			solThrow(CommandLineExecutionError, "Documentation parsing failed.");
		}
		else
		{
			m_hasOutput = true;
			formatter.printExceptionInformation(_error, _error.type());
			solThrow(CommandLineExecutionError, "");
		}
	}
}

void CommandLineInterface::handleCombinedJSON()
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.combinedJsonRequests.has_value())
		return;

	Json::Value output(Json::objectValue);

	output[g_strVersion] = frontend::VersionString;
	vector<string> contracts = m_compiler->contractNames();

	if (!contracts.empty())
		output[g_strContracts] = Json::Value(Json::objectValue);
	for (string const& contractName: contracts)
	{
		Json::Value& contractData = output[g_strContracts][contractName] = Json::objectValue;
		if (m_options.compiler.combinedJsonRequests->abi)
			contractData[g_strAbi] = m_compiler->contractABI(contractName);
		if (m_options.compiler.combinedJsonRequests->metadata)
			contractData["metadata"] = m_compiler->metadata(contractName);
		if (m_options.compiler.combinedJsonRequests->binary && m_compiler->compilationSuccessful())
			contractData[g_strBinary] = m_compiler->object(contractName).toHex();
		if (m_options.compiler.combinedJsonRequests->binaryRuntime && m_compiler->compilationSuccessful())
			contractData[g_strBinaryRuntime] = m_compiler->runtimeObject(contractName).toHex();
		if (m_options.compiler.combinedJsonRequests->opcodes && m_compiler->compilationSuccessful())
			contractData[g_strOpcodes] = evmasm::disassemble(m_compiler->object(contractName).bytecode);
		if (m_options.compiler.combinedJsonRequests->asm_ && m_compiler->compilationSuccessful())
			contractData[g_strAsm] = m_compiler->assemblyJSON(contractName);
		if (m_options.compiler.combinedJsonRequests->storageLayout && m_compiler->compilationSuccessful())
			contractData[g_strStorageLayout] = m_compiler->storageLayout(contractName);
		if (m_options.compiler.combinedJsonRequests->generatedSources && m_compiler->compilationSuccessful())
			contractData[g_strGeneratedSources] = m_compiler->generatedSources(contractName, false);
		if (m_options.compiler.combinedJsonRequests->generatedSourcesRuntime && m_compiler->compilationSuccessful())
			contractData[g_strGeneratedSourcesRuntime] = m_compiler->generatedSources(contractName, true);
		if (m_options.compiler.combinedJsonRequests->srcMap && m_compiler->compilationSuccessful())
		{
			auto map = m_compiler->sourceMapping(contractName);
			contractData[g_strSrcMap] = map ? *map : "";
		}
		if (m_options.compiler.combinedJsonRequests->srcMapRuntime && m_compiler->compilationSuccessful())
		{
			auto map = m_compiler->runtimeSourceMapping(contractName);
			contractData[g_strSrcMapRuntime] = map ? *map : "";
		}
		if (m_options.compiler.combinedJsonRequests->funDebug && m_compiler->compilationSuccessful())
			contractData[g_strFunDebug] = StandardCompiler::formatFunctionDebugData(
				m_compiler->object(contractName).functionDebugData
			);
		if (m_options.compiler.combinedJsonRequests->funDebugRuntime && m_compiler->compilationSuccessful())
			contractData[g_strFunDebugRuntime] = StandardCompiler::formatFunctionDebugData(
				m_compiler->runtimeObject(contractName).functionDebugData
			);
		if (m_options.compiler.combinedJsonRequests->signatureHashes)
			contractData[g_strSignatureHashes] = m_compiler->interfaceSymbols(contractName)["methods"];
		if (m_options.compiler.combinedJsonRequests->natspecDev)
			contractData[g_strNatspecDev] = m_compiler->natspecDev(contractName);
		if (m_options.compiler.combinedJsonRequests->natspecUser)
			contractData[g_strNatspecUser] = m_compiler->natspecUser(contractName);
	}

	bool needsSourceList =
		m_options.compiler.combinedJsonRequests->ast ||
		m_options.compiler.combinedJsonRequests->srcMap ||
		m_options.compiler.combinedJsonRequests->srcMapRuntime;
	if (needsSourceList)
	{
		// Indices into this array are used to abbreviate source names in source locations.
		output[g_strSourceList] = Json::Value(Json::arrayValue);

		for (auto const& source: m_compiler->sourceNames())
			output[g_strSourceList].append(source);
	}

	if (m_options.compiler.combinedJsonRequests->ast)
	{
		output[g_strSources] = Json::Value(Json::objectValue);
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			ASTJsonExporter converter(m_compiler->state(), m_compiler->sourceIndices());
			output[g_strSources][sourceCode.first] = Json::Value(Json::objectValue);
			output[g_strSources][sourceCode.first]["AST"] = converter.toJson(m_compiler->ast(sourceCode.first));
		}
	}

	string json = jsonPrint(removeNullMembers(std::move(output)), m_options.formatting.json);
	if (!m_options.output.dir.empty())
		createJson("combined", json);
	else
		sout() << json << endl;
}

void CommandLineInterface::handleAst()
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	if (!m_options.compiler.outputs.astCompactJson)
		return;

	vector<ASTNode const*> asts;
	for (auto const& sourceCode: m_fileReader.sourceUnits())
		asts.push_back(&m_compiler->ast(sourceCode.first));

	if (!m_options.output.dir.empty())
	{
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			stringstream data;
			string postfix = "";
			ASTJsonExporter(m_compiler->state(), m_compiler->sourceIndices()).print(data, m_compiler->ast(sourceCode.first), m_options.formatting.json);
			postfix += "_json";
			boost::filesystem::path path(sourceCode.first);
			createFile(path.filename().string() + postfix + ".ast", data.str());
		}
	}
	else
	{
		sout() << "JSON AST (compact format):" << endl << endl;
		for (auto const& sourceCode: m_fileReader.sourceUnits())
		{
			sout() << endl << "======= " << sourceCode.first << " =======" << endl;
			ASTJsonExporter(m_compiler->state(), m_compiler->sourceIndices()).print(sout(), m_compiler->ast(sourceCode.first), m_options.formatting.json);
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
	solAssert(m_options.input.mode == InputMode::Linker, "");

	// Map from how the libraries will be named inside the bytecode to their addresses.
	map<string, h160> librariesReplacements;
	int const placeholderSize = 40; // 20 bytes or 40 hex characters
	for (auto const& library: m_options.linker.libraries)
	{
		string const& name = library.first;
		// Library placeholders are 40 hex digits (20 bytes) that start and end with '__'.
		// This leaves 36 characters for the library identifier. The identifier used to
		// be just the cropped or '_'-padded library name, but this changed to
		// the cropped hex representation of the hash of the library name.
		// We support both ways of linking here.
		librariesReplacements["__" + evmasm::LinkerObject::libraryPlaceholder(name) + "__"] = library.second;

		string replacement = "__";
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
					"Error in binary object file " + src.first + " at position " + to_string(it - src.second.begin()) + "\n" +
					'"' + string(it, it + min(placeholderSize, static_cast<int>(end - it))) + "\" is not a valid link reference."
				);

			string foundPlaceholder(it, it + placeholderSize);
			if (librariesReplacements.count(foundPlaceholder))
			{
				string hexStr(util::toHex(librariesReplacements.at(foundPlaceholder).asBytes()));
				copy(hexStr.begin(), hexStr.end(), it);
			}
			else
				serr() << "Reference \"" << foundPlaceholder << "\" in file \"" << src.first << "\" still unresolved." << endl;
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
	solAssert(m_options.input.mode == InputMode::Linker, "");

	for (auto const& src: m_fileReader.sourceUnits())
		if (src.first == g_stdinFileName)
			sout() << src.second << endl;
		else
		{
			ofstream outFile(src.first);
			outFile << src.second;
			if (!outFile)
				solThrow(CommandLineOutputError, "Could not write to file " + src.first + ". Aborting.");
		}
	sout() << "Linking completed." << endl;
}

string CommandLineInterface::libraryPlaceholderHint(string const& _libraryName)
{
	return "// " + evmasm::LinkerObject::libraryPlaceholder(_libraryName) + " -> " + _libraryName;
}

string CommandLineInterface::objectWithLinkRefsHex(evmasm::LinkerObject const& _obj)
{
	string out = _obj.toHex();
	if (!_obj.linkReferences.empty())
	{
		out += "\n";
		for (auto const& linkRef: _obj.linkReferences)
			out += "\n" + libraryPlaceholderHint(linkRef.second);
	}
	return out;
}

void CommandLineInterface::assemble(yul::YulStack::Language _language, yul::YulStack::Machine _targetMachine)
{
	solAssert(m_options.input.mode == InputMode::Assembler, "");

	bool successful = true;
	map<string, yul::YulStack> yulStacks;
	for (auto const& src: m_fileReader.sourceUnits())
	{
		// --no-optimize-yul option is not accepted in assembly mode.
		solAssert(!m_options.optimizer.noOptimizeYul, "");

		auto& stack = yulStacks[src.first] = yul::YulStack(
			m_options.output.evmVersion,
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
		string machine =
			_targetMachine == yul::YulStack::Machine::EVM ? "EVM" :
			"Ewasm";
		sout() << endl << "======= " << src.first << " (" << machine << ") =======" << endl;

		yul::YulStack& stack = yulStacks[src.first];

		if (m_options.compiler.outputs.irOptimized)
		{
			// NOTE: This actually outputs unoptimized code when the optimizer is disabled but
			// 'ir' output in StandardCompiler works the same way.
			sout() << endl << "Pretty printed source:" << endl;
			sout() << stack.print() << endl;
		}

		if (_language != yul::YulStack::Language::Ewasm && _targetMachine == yul::YulStack::Machine::Ewasm)
		{
			stack.translate(yul::YulStack::Language::Ewasm);
			stack.optimize();

			if (m_options.compiler.outputs.ewasmIR)
			{
				sout() << endl << "==========================" << endl;
				sout() << endl << "Translated source:" << endl;
				sout() << stack.print() << endl;
			}
		}

		yul::MachineAssemblyObject object;
		object = stack.assemble(_targetMachine);
		object.bytecode->link(m_options.linker.libraries);

		if (m_options.compiler.outputs.binary)
		{
			sout() << endl << "Binary representation:" << endl;
			if (object.bytecode)
				sout() << object.bytecode->toHex() << endl;
			else
				serr() << "No binary representation found." << endl;
		}

		solAssert(_targetMachine == yul::YulStack::Machine::Ewasm || _targetMachine == yul::YulStack::Machine::EVM, "");
		if (
			(_targetMachine == yul::YulStack::Machine::EVM && m_options.compiler.outputs.asm_) ||
			(_targetMachine == yul::YulStack::Machine::Ewasm && m_options.compiler.outputs.ewasm)
		)
		{
			sout() << endl << "Text representation:" << endl;
			if (!object.assembly.empty())
				sout() << object.assembly << endl;
			else
				serr() << "No text representation found." << endl;
		}
	}
}

void CommandLineInterface::outputCompilationResults()
{
	solAssert(m_options.input.mode == InputMode::Compiler || m_options.input.mode == InputMode::CompilerWithASTImport, "");

	handleCombinedJSON();

	// do we need AST output?
	handleAst();

	if (
		!m_compiler->compilationSuccessful() &&
		m_options.output.stopAfter == CompilerStack::State::CompilationSuccessful
	)
	{
		serr() << endl << "Compilation halted after AST generation due to errors." << endl;
		return;
	}

	vector<string> contracts = m_compiler->contractNames();
	for (string const& contract: contracts)
	{
		if (needsHumanTargetedStdout(m_options))
			sout() << endl << "======= " << contract << " =======" << endl;

		// do we need EVM assembly?
		if (m_options.compiler.outputs.asm_ || m_options.compiler.outputs.asmJson)
		{
			string ret;
			if (m_options.compiler.outputs.asmJson)
				ret = util::jsonPrint(removeNullMembers(m_compiler->assemblyJSON(contract)), m_options.formatting.json);
			else
				ret = m_compiler->assemblyString(contract, m_fileReader.sourceUnits());

			if (!m_options.output.dir.empty())
				createFile(m_compiler->filesystemFriendlyName(contract) + (m_options.compiler.outputs.asmJson ? "_evm.json" : ".evm"), ret);
			else
				sout() << "EVM assembly:" << endl << ret << endl;
		}

		if (m_options.compiler.estimateGas)
			handleGasEstimation(contract);

		handleBytecode(contract);
		handleIR(contract);
		handleIROptimized(contract);
		handleEwasm(contract);
		handleSignatureHashes(contract);
		handleMetadata(contract);
		handleABI(contract);
		handleStorageLayout(contract);
		handleNatspec(true, contract);
		handleNatspec(false, contract);
	} // end of contracts iteration

	if (!m_hasOutput)
	{
		if (!m_options.output.dir.empty())
			sout() << "Compiler run successful. Artifact(s) can be found in directory " << m_options.output.dir << "." << endl;
		else
			serr() << "Compiler run successful, no output requested." << endl;
	}
}

}
