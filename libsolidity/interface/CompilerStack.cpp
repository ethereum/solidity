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
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */


#include <libsolidity/interface/CompilerStack.h>

#include <libsolidity/analysis/ControlFlowAnalyzer.h>
#include <libsolidity/analysis/ControlFlowGraph.h>
#include <libsolidity/analysis/ContractLevelChecker.h>
#include <libsolidity/analysis/DocStringAnalyser.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/PostTypeChecker.h>
#include <libsolidity/analysis/SemVerHandler.h>
#include <libsolidity/analysis/StaticAnalyzer.h>
#include <libsolidity/analysis/SyntaxChecker.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/analysis/ViewPureChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/formal/SMTChecker.h>
#include <libsolidity/interface/ABI.h>
#include <libsolidity/interface/Natspec.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/interface/Version.h>
#include <libsolidity/parsing/Parser.h>

#include <libsolidity/codegen/ir/IRGenerator.h>

#include <libyul/YulString.h>

#include <liblangutil/Scanner.h>

#include <libevmasm/Exceptions.h>

#include <libdevcore/SwarmHash.h>
#include <libdevcore/JSON.h>

#include <json/json.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

static int g_compilerStackCounts = 0;

CompilerStack::CompilerStack(ReadCallback::Callback const& _readFile):
	m_readFile{_readFile},
	m_generateIR{false},
	m_errorList{},
	m_errorReporter{m_errorList}
{
	// Because TypeProvider is currently a singleton API, we must ensure that
	// no more than one entity is actually using it at a time.
	solAssert(g_compilerStackCounts == 0, "You shall not have another CompilerStack aside me.");
	++g_compilerStackCounts;
}

CompilerStack::~CompilerStack()
{
	--g_compilerStackCounts;
	TypeProvider::reset();
}

boost::optional<CompilerStack::Remapping> CompilerStack::parseRemapping(string const& _remapping)
{
	auto eq = find(_remapping.begin(), _remapping.end(), '=');
	if (eq == _remapping.end())
		return {};

	auto colon = find(_remapping.begin(), eq, ':');

	Remapping r;

	r.context = colon == eq ? string() : string(_remapping.begin(), colon);
	r.prefix = colon == eq ? string(_remapping.begin(), eq) : string(colon + 1, eq);
	r.target = string(eq + 1, _remapping.end());

	if (r.prefix.empty())
		return {};

	return r;
}

void CompilerStack::setRemappings(vector<Remapping> const& _remappings)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set remappings before parsing."));
	for (auto const& remapping: _remappings)
		solAssert(!remapping.prefix.empty(), "");
	m_remappings = _remappings;
}

void CompilerStack::setEVMVersion(langutil::EVMVersion _version)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set EVM version before parsing."));
	m_evmVersion = _version;
}

void CompilerStack::setLibraries(std::map<std::string, h160> const& _libraries)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set libraries before parsing."));
	m_libraries = _libraries;
}

void CompilerStack::setOptimiserSettings(bool _optimize, unsigned _runs)
{
	OptimiserSettings settings = _optimize ? OptimiserSettings::standard() : OptimiserSettings::minimal();
	settings.expectedExecutionsPerDeployment = _runs;
	setOptimiserSettings(std::move(settings));
}

void CompilerStack::setOptimiserSettings(OptimiserSettings _settings)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set optimiser settings before parsing."));
	m_optimiserSettings = std::move(_settings);
}

void CompilerStack::useMetadataLiteralSources(bool _metadataLiteralSources)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set use literal sources before parsing."));
	m_metadataLiteralSources = _metadataLiteralSources;
}

void CompilerStack::addSMTLib2Response(h256 const& _hash, string const& _response)
{
	if (m_stackState >= ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must add SMTLib2 responses before parsing."));
	m_smtlib2Responses[_hash] = _response;
}

void CompilerStack::reset(bool _keepSettings)
{
	m_stackState = Empty;
	m_sources.clear();
	m_smtlib2Responses.clear();
	m_unhandledSMTLib2Queries.clear();
	if (!_keepSettings)
	{
		m_remappings.clear();
		m_libraries.clear();
		m_evmVersion = langutil::EVMVersion();
		m_generateIR = false;
		m_optimiserSettings = OptimiserSettings::minimal();
		m_metadataLiteralSources = false;
	}
	m_globalContext.reset();
	m_scopes.clear();
	m_sourceOrder.clear();
	m_contracts.clear();
	m_errorReporter.clear();
	TypeProvider::reset();
}

void CompilerStack::setSources(StringMap const& _sources)
{
	if (m_stackState == SourcesSet)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Cannot change sources once set."));
	if (m_stackState != Empty)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must set sources before parsing."));
	for (auto const& source: _sources)
		m_sources[source.first].scanner = make_shared<Scanner>(CharStream(/*content*/source.second, /*name*/source.first));
	m_stackState = SourcesSet;
}

bool CompilerStack::parse()
{
	if (m_stackState != SourcesSet)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must call parse only after the SourcesSet state."));
	m_errorReporter.clear();
	ASTNode::resetID();

	if (SemVerVersion{string(VersionString)}.isPrerelease())
		m_errorReporter.warning("This is a pre-release compiler version, please do not use it in production.");

	if (m_optimiserSettings.runYulOptimiser)
		m_errorReporter.warning(
			"The Yul optimiser is still experimental. "
			"Do not use it in production unless correctness of generated code is verified with extensive tests."
		);

	vector<string> sourcesToParse;
	for (auto const& s: m_sources)
		sourcesToParse.push_back(s.first);
	for (size_t i = 0; i < sourcesToParse.size(); ++i)
	{
		string const& path = sourcesToParse[i];
		Source& source = m_sources[path];
		source.scanner->reset();
		source.ast = Parser(m_errorReporter).parse(source.scanner);
		if (!source.ast)
			solAssert(!Error::containsOnlyWarnings(m_errorReporter.errors()), "Parser returned null but did not report error.");
		else
		{
			source.ast->annotation().path = path;
			for (auto const& newSource: loadMissingSources(*source.ast, path))
			{
				string const& newPath = newSource.first;
				string const& newContents = newSource.second;
				m_sources[newPath].scanner = make_shared<Scanner>(CharStream(newContents, newPath));
				sourcesToParse.push_back(newPath);
			}
		}
	}
	if (Error::containsOnlyWarnings(m_errorReporter.errors()))
	{
		m_stackState = ParsingSuccessful;
		return true;
	}
	else
		return false;
}

bool CompilerStack::analyze()
{
	if (m_stackState != ParsingSuccessful || m_stackState >= AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Must call analyze only after parsing was successful."));
	resolveImports();

	bool noErrors = true;

	try {
		SyntaxChecker syntaxChecker(m_errorReporter);
		for (Source const* source: m_sourceOrder)
			if (!syntaxChecker.checkSyntax(*source->ast))
				noErrors = false;

		DocStringAnalyser docStringAnalyser(m_errorReporter);
		for (Source const* source: m_sourceOrder)
			if (!docStringAnalyser.analyseDocStrings(*source->ast))
				noErrors = false;

		m_globalContext = make_shared<GlobalContext>();
		NameAndTypeResolver resolver(m_globalContext->declarations(), m_scopes, m_errorReporter);
		for (Source const* source: m_sourceOrder)
			if (!resolver.registerDeclarations(*source->ast))
				return false;

		map<string, SourceUnit const*> sourceUnitsByName;
		for (auto& source: m_sources)
			sourceUnitsByName[source.first] = source.second.ast.get();
		for (Source const* source: m_sourceOrder)
			if (!resolver.performImports(*source->ast, sourceUnitsByName))
				return false;

		// This is the main name and type resolution loop. Needs to be run for every contract, because
		// the special variables "this" and "super" must be set appropriately.
		for (Source const* source: m_sourceOrder)
			for (ASTPointer<ASTNode> const& node: source->ast->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
				{
					m_globalContext->setCurrentContract(*contract);
					if (!resolver.updateDeclaration(*m_globalContext->currentThis())) return false;
					if (!resolver.updateDeclaration(*m_globalContext->currentSuper())) return false;
					if (!resolver.resolveNamesAndTypes(*contract)) return false;

					// Note that we now reference contracts by their fully qualified names, and
					// thus contracts can only conflict if declared in the same source file.  This
					// already causes a double-declaration error elsewhere, so we do not report
					// an error here and instead silently drop any additional contracts we find.
					if (m_contracts.find(contract->fullyQualifiedName()) == m_contracts.end())
						m_contracts[contract->fullyQualifiedName()].contract = contract;
				}

		// Next, we check inheritance, overrides, function collisions and other things at
		// contract or function level.
		// This also calculates whether a contract is abstract, which is needed by the
		// type checker.
		ContractLevelChecker contractLevelChecker(m_errorReporter);
		for (Source const* source: m_sourceOrder)
			for (ASTPointer<ASTNode> const& node: source->ast->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
					if (!contractLevelChecker.check(*contract))
						noErrors = false;

		// New we run full type checks that go down to the expression level. This
		// cannot be done earlier, because we need cross-contract types and information
		// about whether a contract is abstract for the `new` expression.
		// This populates the `type` annotation for all expressions.
		//
		// Note: this does not resolve overloaded functions. In order to do that, types of arguments are needed,
		// which is only done one step later.
		TypeChecker typeChecker(m_evmVersion, m_errorReporter);
		for (Source const* source: m_sourceOrder)
			for (ASTPointer<ASTNode> const& node: source->ast->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
					if (!typeChecker.checkTypeRequirements(*contract))
						noErrors = false;

		if (noErrors)
		{
			// Checks that can only be done when all types of all AST nodes are known.
			PostTypeChecker postTypeChecker(m_errorReporter);
			for (Source const* source: m_sourceOrder)
				if (!postTypeChecker.check(*source->ast))
					noErrors = false;
		}

		if (noErrors)
		{
			// Control flow graph generator and analyzer. It can check for issues such as
			// variable is used before it is assigned to.
			CFG cfg(m_errorReporter);
			for (Source const* source: m_sourceOrder)
				if (!cfg.constructFlow(*source->ast))
					noErrors = false;

			if (noErrors)
			{
				ControlFlowAnalyzer controlFlowAnalyzer(cfg, m_errorReporter);
				for (Source const* source: m_sourceOrder)
					if (!controlFlowAnalyzer.analyze(*source->ast))
						noErrors = false;
			}
		}

		if (noErrors)
		{
			// Checks for common mistakes. Only generates warnings.
			StaticAnalyzer staticAnalyzer(m_errorReporter);
			for (Source const* source: m_sourceOrder)
				if (!staticAnalyzer.analyze(*source->ast))
					noErrors = false;
		}

		if (noErrors)
		{
			// Check for state mutability in every function.
			vector<ASTPointer<ASTNode>> ast;
			for (Source const* source: m_sourceOrder)
				ast.push_back(source->ast);

			if (!ViewPureChecker(ast, m_errorReporter).check())
				noErrors = false;
		}

		if (noErrors)
		{
			SMTChecker smtChecker(m_errorReporter, m_smtlib2Responses);
			for (Source const* source: m_sourceOrder)
				smtChecker.analyze(*source->ast, source->scanner);
			m_unhandledSMTLib2Queries += smtChecker.unhandledQueries();
		}
	}
	catch(FatalError const&)
	{
		if (m_errorReporter.errors().empty())
			throw; // Something is weird here, rather throw again.
		noErrors = false;
	}

	if (noErrors)
	{
		m_stackState = AnalysisSuccessful;
		return true;
	}
	else
		return false;
}

bool CompilerStack::parseAndAnalyze()
{
	return parse() && analyze();
}

bool CompilerStack::isRequestedContract(ContractDefinition const& _contract) const
{
	return
		m_requestedContractNames.empty() ||
		m_requestedContractNames.count(_contract.fullyQualifiedName()) ||
		m_requestedContractNames.count(_contract.name());
}

bool CompilerStack::compile()
{
	if (m_stackState < AnalysisSuccessful)
		if (!parseAndAnalyze())
			return false;

	// Only compile contracts individually which have been requested.
	map<ContractDefinition const*, shared_ptr<Compiler const>> otherCompilers;
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				if (isRequestedContract(*contract))
				{
					compileContract(*contract, otherCompilers);
					if (m_generateIR)
						generateIR(*contract);
				}
	m_stackState = CompilationSuccessful;
	this->link();
	return true;
}

void CompilerStack::link()
{
	solAssert(m_stackState >= CompilationSuccessful, "");
	for (auto& contract: m_contracts)
	{
		contract.second.object.link(m_libraries);
		contract.second.runtimeObject.link(m_libraries);
	}
}

vector<string> CompilerStack::contractNames() const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	vector<string> contractNames;
	for (auto const& contract: m_contracts)
		contractNames.push_back(contract.first);
	return contractNames;
}

string const CompilerStack::lastContractName() const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	// try to find some user-supplied contract
	string contractName;
	for (auto const& it: m_sources)
		for (ASTPointer<ASTNode> const& node: it.second.ast->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				contractName = contract->fullyQualifiedName();
	return contractName;
}

eth::AssemblyItems const* CompilerStack::assemblyItems(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& currentContract = contract(_contractName);
	return currentContract.compiler ? &contract(_contractName).compiler->assemblyItems() : nullptr;
}

eth::AssemblyItems const* CompilerStack::runtimeAssemblyItems(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& currentContract = contract(_contractName);
	return currentContract.compiler ? &contract(_contractName).compiler->runtimeAssemblyItems() : nullptr;
}

string const* CompilerStack::sourceMapping(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& c = contract(_contractName);
	if (!c.sourceMapping)
	{
		if (auto items = assemblyItems(_contractName))
			c.sourceMapping.reset(new string(computeSourceMapping(*items)));
	}
	return c.sourceMapping.get();
}

string const* CompilerStack::runtimeSourceMapping(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& c = contract(_contractName);
	if (!c.runtimeSourceMapping)
	{
		if (auto items = runtimeAssemblyItems(_contractName))
			c.runtimeSourceMapping.reset(new string(computeSourceMapping(*items)));
	}
	return c.runtimeSourceMapping.get();
}

std::string const CompilerStack::filesystemFriendlyName(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));

	// Look up the contract (by its fully-qualified name)
	Contract const& matchContract = m_contracts.at(_contractName);
	// Check to see if it could collide on name
	for (auto const& contract: m_contracts)
	{
		if (contract.second.contract->name() == matchContract.contract->name() &&
				contract.second.contract != matchContract.contract)
		{
			// If it does, then return its fully-qualified name, made fs-friendly
			std::string friendlyName = boost::algorithm::replace_all_copy(_contractName, "/", "_");
			boost::algorithm::replace_all(friendlyName, ":", "_");
			boost::algorithm::replace_all(friendlyName, ".", "_");
			return friendlyName;
		}
	}
	// If no collision, return the contract's name
	return matchContract.contract->name();
}

string const& CompilerStack::yulIR(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return contract(_contractName).yulIR;
}

string const& CompilerStack::yulIROptimized(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return contract(_contractName).yulIROptimized;
}

eth::LinkerObject const& CompilerStack::object(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return contract(_contractName).object;
}

eth::LinkerObject const& CompilerStack::runtimeObject(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return contract(_contractName).runtimeObject;
}

/// TODO: cache this string
string CompilerStack::assemblyString(string const& _contractName, StringMap _sourceCodes) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& currentContract = contract(_contractName);
	if (currentContract.compiler)
		return currentContract.compiler->assemblyString(_sourceCodes);
	else
		return string();
}

/// TODO: cache the JSON
Json::Value CompilerStack::assemblyJSON(string const& _contractName, StringMap _sourceCodes) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	Contract const& currentContract = contract(_contractName);
	if (currentContract.compiler)
		return currentContract.compiler->assemblyJSON(_sourceCodes);
	else
		return Json::Value();
}

vector<string> CompilerStack::sourceNames() const
{
	vector<string> names;
	for (auto const& s: m_sources)
		names.push_back(s.first);
	return names;
}

map<string, unsigned> CompilerStack::sourceIndices() const
{
	map<string, unsigned> indices;
	unsigned index = 0;
	for (auto const& s: m_sources)
		indices[s.first] = index++;
	return indices;
}

Json::Value const& CompilerStack::contractABI(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	return contractABI(contract(_contractName));
}

Json::Value const& CompilerStack::contractABI(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.abi)
		_contract.abi.reset(new Json::Value(ABI::generate(*_contract.contract)));

	return *_contract.abi;
}

Json::Value const& CompilerStack::natspecUser(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	return natspecUser(contract(_contractName));
}

Json::Value const& CompilerStack::natspecUser(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.userDocumentation)
		_contract.userDocumentation.reset(new Json::Value(Natspec::userDocumentation(*_contract.contract)));

	return *_contract.userDocumentation;
}

Json::Value const& CompilerStack::natspecDev(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	return natspecDev(contract(_contractName));
}

Json::Value const& CompilerStack::natspecDev(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.devDocumentation)
		_contract.devDocumentation.reset(new Json::Value(Natspec::devDocumentation(*_contract.contract)));

	return *_contract.devDocumentation;
}

Json::Value CompilerStack::methodIdentifiers(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	Json::Value methodIdentifiers(Json::objectValue);
	for (auto const& it: contractDefinition(_contractName).interfaceFunctions())
		methodIdentifiers[it.second->externalSignature()] = it.first.hex();
	return methodIdentifiers;
}

string const& CompilerStack::metadata(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	return metadata(contract(_contractName));
}

string const& CompilerStack::metadata(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	solAssert(_contract.contract, "");

	// cache the result
	if (!_contract.metadata)
		_contract.metadata.reset(new string(createMetadata(_contract)));

	return *_contract.metadata;
}

Scanner const& CompilerStack::scanner(string const& _sourceName) const
{
	if (m_stackState < SourcesSet)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No sources set."));

	return *source(_sourceName).scanner;
}

SourceUnit const& CompilerStack::ast(string const& _sourceName) const
{
	if (m_stackState < ParsingSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	return *source(_sourceName).ast;
}

ContractDefinition const& CompilerStack::contractDefinition(string const& _contractName) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Analysis was not successful."));

	return *contract(_contractName).contract;
}

size_t CompilerStack::functionEntryPoint(
	std::string const& _contractName,
	FunctionDefinition const& _function
) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	shared_ptr<Compiler> const& compiler = contract(_contractName).compiler;
	if (!compiler)
		return 0;
	eth::AssemblyItem tag = compiler->functionEntryLabel(_function);
	if (tag.type() == eth::UndefinedItem)
		return 0;
	eth::AssemblyItems const& items = compiler->runtimeAssemblyItems();
	for (size_t i = 0; i < items.size(); ++i)
		if (items.at(i).type() == eth::Tag && items.at(i).data() == tag.data())
			return i;
	return 0;
}

tuple<int, int, int, int> CompilerStack::positionFromSourceLocation(SourceLocation const& _sourceLocation) const
{
	int startLine;
	int startColumn;
	int endLine;
	int endColumn;
	tie(startLine, startColumn) = scanner(_sourceLocation.source->name()).translatePositionToLineColumn(_sourceLocation.start);
	tie(endLine, endColumn) = scanner(_sourceLocation.source->name()).translatePositionToLineColumn(_sourceLocation.end);

	return make_tuple(++startLine, ++startColumn, ++endLine, ++endColumn);
}


h256 const& CompilerStack::Source::keccak256() const
{
	if (keccak256HashCached == h256{})
		keccak256HashCached = dev::keccak256(scanner->source());
	return keccak256HashCached;
}

h256 const& CompilerStack::Source::swarmHash() const
{
	if (swarmHashCached == h256{})
		swarmHashCached = dev::swarmHash(scanner->source());
	return swarmHashCached;
}


StringMap CompilerStack::loadMissingSources(SourceUnit const& _ast, std::string const& _sourcePath)
{
	solAssert(m_stackState < ParsingSuccessful, "");
	StringMap newSources;
	for (auto const& node: _ast.nodes())
		if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
		{
			solAssert(!import->path().empty(), "Import path cannot be empty.");

			string importPath = dev::absolutePath(import->path(), _sourcePath);
			// The current value of `path` is the absolute path as seen from this source file.
			// We first have to apply remappings before we can store the actual absolute path
			// as seen globally.
			importPath = applyRemapping(importPath, _sourcePath);
			import->annotation().absolutePath = importPath;
			if (m_sources.count(importPath) || newSources.count(importPath))
				continue;

			ReadCallback::Result result{false, string("File not supplied initially.")};
			if (m_readFile)
				result = m_readFile(importPath);

			if (result.success)
				newSources[importPath] = result.responseOrErrorMessage;
			else
			{
				m_errorReporter.parserError(
					import->location(),
					string("Source \"" + importPath + "\" not found: " + result.responseOrErrorMessage)
				);
				continue;
			}
		}
	return newSources;
}

string CompilerStack::applyRemapping(string const& _path, string const& _context)
{
	solAssert(m_stackState < ParsingSuccessful, "");
	// Try to find the longest prefix match in all remappings that are active in the current context.
	auto isPrefixOf = [](string const& _a, string const& _b)
	{
		if (_a.length() > _b.length())
			return false;
		return std::equal(_a.begin(), _a.end(), _b.begin());
	};

	size_t longestPrefix = 0;
	size_t longestContext = 0;
	string bestMatchTarget;

	for (auto const& redir: m_remappings)
	{
		string context = dev::sanitizePath(redir.context);
		string prefix = dev::sanitizePath(redir.prefix);

		// Skip if current context is closer
		if (context.length() < longestContext)
			continue;
		// Skip if redir.context is not a prefix of _context
		if (!isPrefixOf(context, _context))
			continue;
		// Skip if we already have a closer prefix match.
		if (prefix.length() < longestPrefix && context.length() == longestContext)
			continue;
		// Skip if the prefix does not match.
		if (!isPrefixOf(prefix, _path))
			continue;

		longestContext = context.length();
		longestPrefix = prefix.length();
		bestMatchTarget = dev::sanitizePath(redir.target);
	}
	string path = bestMatchTarget;
	path.append(_path.begin() + longestPrefix, _path.end());
	return path;
}

void CompilerStack::resolveImports()
{
	solAssert(m_stackState == ParsingSuccessful, "");

	// topological sorting (depth first search) of the import graph, cutting potential cycles
	vector<Source const*> sourceOrder;
	set<Source const*> sourcesSeen;

	function<void(Source const*)> toposort = [&](Source const* _source)
	{
		if (sourcesSeen.count(_source))
			return;
		sourcesSeen.insert(_source);
		for (ASTPointer<ASTNode> const& node: _source->ast->nodes())
			if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
			{
				string const& path = import->annotation().absolutePath;
				solAssert(!path.empty(), "");
				solAssert(m_sources.count(path), "");
				import->annotation().sourceUnit = m_sources[path].ast.get();
				toposort(&m_sources[path]);
			}
		sourceOrder.push_back(_source);
	};

	for (auto const& sourcePair: m_sources)
		toposort(&sourcePair.second);

	swap(m_sourceOrder, sourceOrder);
}

namespace
{
bool onlySafeExperimentalFeaturesActivated(set<ExperimentalFeature> const& features)
{
	for (auto const feature: features)
		if (!ExperimentalFeatureOnlyAnalysis.count(feature))
			return false;
	return true;
}
}

void CompilerStack::compileContract(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, shared_ptr<Compiler const>>& _otherCompilers
)
{
	solAssert(m_stackState >= AnalysisSuccessful, "");

	if (_otherCompilers.count(&_contract) || !_contract.canBeDeployed())
		return;
	for (auto const* dependency: _contract.annotation().contractDependencies)
		compileContract(*dependency, _otherCompilers);

	Contract& compiledContract = m_contracts.at(_contract.fullyQualifiedName());

	shared_ptr<Compiler> compiler = make_shared<Compiler>(m_evmVersion, m_optimiserSettings);
	compiledContract.compiler = compiler;

	bytes cborEncodedMetadata = createCBORMetadata(
		metadata(compiledContract),
		!onlySafeExperimentalFeaturesActivated(_contract.sourceUnit().annotation().experimentalFeatures)
	);

	try
	{
		// Run optimiser and compile the contract.
		compiler->compileContract(_contract, _otherCompilers, cborEncodedMetadata);
	}
	catch(eth::OptimizerException const&)
	{
		solAssert(false, "Optimizer exception during compilation");
	}

	try
	{
		// Assemble deployment (incl. runtime)  object.
		compiledContract.object = compiler->assembledObject();
	}
	catch(eth::AssemblyException const&)
	{
		solAssert(false, "Assembly exception for bytecode");
	}

	try
	{
		// Assemble runtime object.
		compiledContract.runtimeObject = compiler->runtimeObject();
	}
	catch(eth::AssemblyException const&)
	{
		solAssert(false, "Assembly exception for deployed bytecode");
	}

	_otherCompilers[compiledContract.contract] = compiler;
}

void CompilerStack::generateIR(ContractDefinition const& _contract)
{
	solAssert(m_stackState >= AnalysisSuccessful, "");

	if (!_contract.canBeDeployed())
		return;

	Contract& compiledContract = m_contracts.at(_contract.fullyQualifiedName());
	if (!compiledContract.yulIR.empty())
		return;

	for (auto const* dependency: _contract.annotation().contractDependencies)
		generateIR(*dependency);

	IRGenerator generator(m_evmVersion, m_optimiserSettings);
	tie(compiledContract.yulIR, compiledContract.yulIROptimized) = generator.run(_contract);
}

CompilerStack::Contract const& CompilerStack::contract(string const& _contractName) const
{
	solAssert(m_stackState >= AnalysisSuccessful, "");

	auto it = m_contracts.find(_contractName);
	if (it != m_contracts.end())
		return it->second;

	// To provide a measure of backward-compatibility, if a contract is not located by its
	// fully-qualified name, a lookup will be attempted purely on the contract's name to see
	// if anything will satisfy.
	if (_contractName.find(':') == string::npos)
	{
		for (auto const& contractEntry: m_contracts)
		{
			stringstream ss;
			ss.str(contractEntry.first);
			// All entries are <source>:<contract>
			string source;
			string foundName;
			getline(ss, source, ':');
			getline(ss, foundName, ':');
			if (foundName == _contractName)
				return contractEntry.second;
		}
	}

	// If we get here, both lookup methods failed.
	BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Contract \"" + _contractName + "\" not found."));
}

CompilerStack::Source const& CompilerStack::source(string const& _sourceName) const
{
	auto it = m_sources.find(_sourceName);
	if (it == m_sources.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Given source file not found."));

	return it->second;
}

string CompilerStack::createMetadata(Contract const& _contract) const
{
	Json::Value meta;
	meta["version"] = 1;
	meta["language"] = "Solidity";
	meta["compiler"]["version"] = VersionStringStrict;

	/// All the source files (including self), which should be included in the metadata.
	set<string> referencedSources;
	referencedSources.insert(_contract.contract->sourceUnit().annotation().path);
	for (auto const sourceUnit: _contract.contract->sourceUnit().referencedSourceUnits(true))
		referencedSources.insert(sourceUnit->annotation().path);

	meta["sources"] = Json::objectValue;
	for (auto const& s: m_sources)
	{
		if (!referencedSources.count(s.first))
			continue;

		solAssert(s.second.scanner, "Scanner not available");
		meta["sources"][s.first]["keccak256"] = "0x" + toHex(s.second.keccak256().asBytes());
		if (m_metadataLiteralSources)
			meta["sources"][s.first]["content"] = s.second.scanner->source();
		else
		{
			meta["sources"][s.first]["urls"] = Json::arrayValue;
			meta["sources"][s.first]["urls"].append("bzzr://" + toHex(s.second.swarmHash().asBytes()));
		}
	}

	static_assert(sizeof(m_optimiserSettings.expectedExecutionsPerDeployment) <= sizeof(Json::LargestUInt), "Invalid word size.");
	solAssert(static_cast<Json::LargestUInt>(m_optimiserSettings.expectedExecutionsPerDeployment) < std::numeric_limits<Json::LargestUInt>::max(), "");
	meta["settings"]["optimizer"]["runs"] = Json::Value(Json::LargestUInt(m_optimiserSettings.expectedExecutionsPerDeployment));

	/// Backwards compatibility: If set to one of the default settings, do not provide details.
	OptimiserSettings settingsWithoutRuns = m_optimiserSettings;
	// reset to default
	settingsWithoutRuns.expectedExecutionsPerDeployment = OptimiserSettings::minimal().expectedExecutionsPerDeployment;
	if (settingsWithoutRuns == OptimiserSettings::minimal())
		meta["settings"]["optimizer"]["enabled"] = false;
	else if (settingsWithoutRuns == OptimiserSettings::standard())
		meta["settings"]["optimizer"]["enabled"] = true;
	else
	{
		Json::Value details{Json::objectValue};

		details["orderLiterals"] = m_optimiserSettings.runOrderLiterals;
		details["jumpdestRemover"] = m_optimiserSettings.runJumpdestRemover;
		details["peephole"] = m_optimiserSettings.runPeephole;
		details["deduplicate"] = m_optimiserSettings.runDeduplicate;
		details["cse"] = m_optimiserSettings.runCSE;
		details["constantOptimizer"] = m_optimiserSettings.runConstantOptimiser;
		details["yul"] = m_optimiserSettings.runYulOptimiser;
		if (m_optimiserSettings.runYulOptimiser)
		{
			details["yulDetails"] = Json::objectValue;
			details["yulDetails"]["stackAllocation"] = m_optimiserSettings.optimizeStackAllocation;
		}

		meta["settings"]["optimizer"]["details"] = std::move(details);
	}

	meta["settings"]["evmVersion"] = m_evmVersion.name();
	meta["settings"]["compilationTarget"][_contract.contract->sourceUnitName()] =
		_contract.contract->annotation().canonicalName;

	meta["settings"]["remappings"] = Json::arrayValue;
	set<string> remappings;
	for (auto const& r: m_remappings)
		remappings.insert(r.context + ":" + r.prefix + "=" + r.target);
	for (auto const& r: remappings)
		meta["settings"]["remappings"].append(r);

	meta["settings"]["libraries"] = Json::objectValue;
	for (auto const& library: m_libraries)
		meta["settings"]["libraries"][library.first] = "0x" + toHex(library.second.asBytes());

	meta["output"]["abi"] = contractABI(_contract);
	meta["output"]["userdoc"] = natspecUser(_contract);
	meta["output"]["devdoc"] = natspecDev(_contract);

	return jsonCompactPrint(meta);
}

class MetadataCBOREncoder
{
public:
	void pushBytes(string const& key, bytes const& value)
	{
		m_entryCount++;
		pushTextString(key);
		pushByteString(value);
	}

	void pushString(string const& key, string const& value)
	{
		m_entryCount++;
		pushTextString(key);
		pushTextString(value);
	}

	void pushBool(string const& key, bool value)
	{
		m_entryCount++;
		pushTextString(key);
		pushBool(value);
	}

	bytes serialise() const
	{
		unsigned size = m_data.size() + 1;
		solAssert(size <= 0xffff, "Metadata too large.");
		solAssert(m_entryCount <= 0x1f, "Too many map entries.");

		// CBOR fixed-length map
		bytes ret{static_cast<unsigned char>(0xa0 + m_entryCount)};
		// The already encoded key-value pairs
		ret += m_data;
		// 16-bit big endian length
		ret += toCompactBigEndian(size, 2);
		return ret;
	}

private:
	void pushTextString(string const& key)
	{
		unsigned length = key.size();
		if (length < 24)
		{
			m_data += bytes{static_cast<unsigned char>(0x60 + length)};
			m_data += key;
		}
		else if (length <= 256)
		{
			m_data += bytes{0x78, static_cast<unsigned char>(length)};
			m_data += key;
		}
		else
			solAssert(false, "Text string too large.");
	}
	void pushByteString(bytes const& key)
	{
		unsigned length = key.size();
		if (length < 24)
		{
			m_data += bytes{static_cast<unsigned char>(0x40 + length)};
			m_data += key;
		}
		else if (length <= 256)
		{
			m_data += bytes{0x58, static_cast<unsigned char>(length)};
			m_data += key;
		}
		else
			solAssert(false, "Byte string too large.");
	}
	void pushBool(bool value)
	{
		if (value)
			m_data += bytes{0xf5};
		else
			m_data += bytes{0xf4};
	}
	unsigned m_entryCount = 0;
	bytes m_data;
};

bytes CompilerStack::createCBORMetadata(string const& _metadata, bool _experimentalMode)
{
	MetadataCBOREncoder encoder;
	encoder.pushBytes("bzzr0", dev::swarmHash(_metadata).asBytes());
	if (_experimentalMode)
		encoder.pushBool("experimental", true);
	return encoder.serialise();
}

string CompilerStack::computeSourceMapping(eth::AssemblyItems const& _items) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	string ret;
	map<string, unsigned> sourceIndicesMap = sourceIndices();
	int prevStart = -1;
	int prevLength = -1;
	int prevSourceIndex = -1;
	char prevJump = 0;
	for (auto const& item: _items)
	{
		if (!ret.empty())
			ret += ";";

		SourceLocation const& location = item.location();
		int length = location.start != -1 && location.end != -1 ? location.end - location.start : -1;
		int sourceIndex =
			location.source && sourceIndicesMap.count(location.source->name()) ?
			sourceIndicesMap.at(location.source->name()) :
			-1;
		char jump = '-';
		if (item.getJumpType() == eth::AssemblyItem::JumpType::IntoFunction)
			jump = 'i';
		else if (item.getJumpType() == eth::AssemblyItem::JumpType::OutOfFunction)
			jump = 'o';

		unsigned components = 4;
		if (jump == prevJump)
		{
			components--;
			if (sourceIndex == prevSourceIndex)
			{
				components--;
				if (length == prevLength)
				{
					components--;
					if (location.start == prevStart)
						components--;
				}
			}
		}

		if (components-- > 0)
		{
			if (location.start != prevStart)
				ret += to_string(location.start);
			if (components-- > 0)
			{
				ret += ':';
				if (length != prevLength)
					ret += to_string(length);
				if (components-- > 0)
				{
					ret += ':';
					if (sourceIndex != prevSourceIndex)
						ret += to_string(sourceIndex);
					if (components-- > 0)
					{
						ret += ':';
						if (jump != prevJump)
							ret += jump;
					}
				}
			}
		}

		prevStart = location.start;
		prevLength = length;
		prevSourceIndex = sourceIndex;
		prevJump = jump;
	}
	return ret;
}

namespace
{

Json::Value gasToJson(GasEstimator::GasConsumption const& _gas)
{
	if (_gas.isInfinite)
		return Json::Value("infinite");
	else
		return Json::Value(toString(_gas.value));
}

}

Json::Value CompilerStack::gasEstimates(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	if (!assemblyItems(_contractName) && !runtimeAssemblyItems(_contractName))
		return Json::Value();

	using Gas = GasEstimator::GasConsumption;
	GasEstimator gasEstimator(m_evmVersion);
	Json::Value output(Json::objectValue);

	if (eth::AssemblyItems const* items = assemblyItems(_contractName))
	{
		Gas executionGas = gasEstimator.functionalEstimation(*items);
		Gas codeDepositGas{eth::GasMeter::dataGas(runtimeObject(_contractName).bytecode, false)};

		Json::Value creation(Json::objectValue);
		creation["codeDepositCost"] = gasToJson(codeDepositGas);
		creation["executionCost"] = gasToJson(executionGas);
		/// TODO: implement + overload to avoid the need of +=
		executionGas += codeDepositGas;
		creation["totalCost"] = gasToJson(executionGas);
		output["creation"] = creation;
	}

	if (eth::AssemblyItems const* items = runtimeAssemblyItems(_contractName))
	{
		/// External functions
		ContractDefinition const& contract = contractDefinition(_contractName);
		Json::Value externalFunctions(Json::objectValue);
		for (auto it: contract.interfaceFunctions())
		{
			string sig = it.second->externalSignature();
			externalFunctions[sig] = gasToJson(gasEstimator.functionalEstimation(*items, sig));
		}

		if (contract.fallbackFunction())
			/// This needs to be set to an invalid signature in order to trigger the fallback,
			/// without the shortcut (of CALLDATSIZE == 0), and therefore to receive the upper bound.
			/// An empty string ("") would work to trigger the shortcut only.
			externalFunctions[""] = gasToJson(gasEstimator.functionalEstimation(*items, "INVALID"));

		if (!externalFunctions.empty())
			output["external"] = externalFunctions;

		/// Internal functions
		Json::Value internalFunctions(Json::objectValue);
		for (auto const& it: contract.definedFunctions())
		{
			/// Exclude externally visible functions, constructor and the fallback function
			if (it->isPartOfExternalInterface() || it->isConstructor() || it->isFallback())
				continue;

			size_t entry = functionEntryPoint(_contractName, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = gasEstimator.functionalEstimation(*items, entry, *it);

			/// TODO: This could move into a method shared with externalSignature()
			FunctionType type(*it);
			string sig = it->name() + "(";
			auto paramTypes = type.parameterTypes();
			for (auto it = paramTypes.begin(); it != paramTypes.end(); ++it)
				sig += (*it)->toString() + (it + 1 == paramTypes.end() ? "" : ",");
			sig += ")";

			internalFunctions[sig] = gasToJson(gas);
		}

		if (!internalFunctions.empty())
			output["internal"] = internalFunctions;
	}

	return output;
}
