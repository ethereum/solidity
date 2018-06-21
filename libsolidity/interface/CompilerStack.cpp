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

#include <libsolidity/interface/Version.h>
#include <libsolidity/analysis/SemVerHandler.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/analysis/ControlFlowAnalyzer.h>
#include <libsolidity/analysis/ControlFlowGraph.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/analysis/DocStringAnalyser.h>
#include <libsolidity/analysis/StaticAnalyzer.h>
#include <libsolidity/analysis/PostTypeChecker.h>
#include <libsolidity/analysis/SyntaxChecker.h>
#include <libsolidity/analysis/ViewPureChecker.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/formal/SMTChecker.h>
#include <libsolidity/interface/ABI.h>
#include <libsolidity/interface/Natspec.h>
#include <libsolidity/interface/GasEstimator.h>

#include <libevmasm/Exceptions.h>

#include <libdevcore/SwarmHash.h>
#include <libdevcore/JSON.h>

#include <json/json.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

void CompilerStack::setRemappings(vector<string> const& _remappings)
{
	vector<Remapping> remappings;
	for (auto const& remapping: _remappings)
	{
		auto eq = find(remapping.begin(), remapping.end(), '=');
		if (eq == remapping.end())
			continue; // ignore
		auto colon = find(remapping.begin(), eq, ':');
		Remapping r;
		r.context = colon == eq ? string() : string(remapping.begin(), colon);
		r.prefix = colon == eq ? string(remapping.begin(), eq) : string(colon + 1, eq);
		r.target = string(eq + 1, remapping.end());
		remappings.push_back(r);
	}
	swap(m_remappings, remappings);
}

void CompilerStack::setEVMVersion(EVMVersion _version)
{
	solAssert(m_stackState < State::ParsingSuccessful, "Set EVM version after parsing.");
	m_evmVersion = _version;
}

void CompilerStack::reset(bool _keepSources)
{
	if (_keepSources)
	{
		m_stackState = SourcesSet;
		for (auto sourcePair: m_sources)
			sourcePair.second.reset();
	}
	else
	{
		m_stackState = Empty;
		m_sources.clear();
	}
	m_libraries.clear();
	m_evmVersion = EVMVersion();
	m_optimize = false;
	m_optimizeRuns = 200;
	m_globalContext.reset();
	m_scopes.clear();
	m_sourceOrder.clear();
	m_contracts.clear();
	m_errorReporter.clear();
}

bool CompilerStack::addSource(string const& _name, string const& _content, bool _isLibrary)
{
	bool existed = m_sources.count(_name) != 0;
	reset(true);
	m_sources[_name].scanner = make_shared<Scanner>(CharStream(_content), _name);
	m_sources[_name].isLibrary = _isLibrary;
	m_stackState = SourcesSet;
	return existed;
}

bool CompilerStack::parse()
{
	//reset
	if(m_stackState != SourcesSet)
		return false;
	m_errorReporter.clear();
	ASTNode::resetID();

	if (SemVerVersion{string(VersionString)}.isPrerelease())
		m_errorReporter.warning("This is a pre-release compiler version, please do not use it in production.");

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
				m_sources[newPath].scanner = make_shared<Scanner>(CharStream(newContents), newPath);
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
	if (m_stackState != ParsingSuccessful)
		return false;
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

		TypeChecker typeChecker(m_evmVersion, m_errorReporter);
		for (Source const* source: m_sourceOrder)
			for (ASTPointer<ASTNode> const& node: source->ast->nodes())
				if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
					if (!typeChecker.checkTypeRequirements(*contract))
						noErrors = false;

		if (noErrors)
		{
			PostTypeChecker postTypeChecker(m_errorReporter);
			for (Source const* source: m_sourceOrder)
				if (!postTypeChecker.check(*source->ast))
					noErrors = false;
		}

		if (noErrors)
		{
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
			StaticAnalyzer staticAnalyzer(m_errorReporter);
			for (Source const* source: m_sourceOrder)
				if (!staticAnalyzer.analyze(*source->ast))
					noErrors = false;
		}

		if (noErrors)
		{
			vector<ASTPointer<ASTNode>> ast;
			for (Source const* source: m_sourceOrder)
				ast.push_back(source->ast);

			if (!ViewPureChecker(ast, m_errorReporter).check())
				noErrors = false;
		}

		if (noErrors)
		{
			SMTChecker smtChecker(m_errorReporter, m_smtQuery);
			for (Source const* source: m_sourceOrder)
				smtChecker.analyze(*source->ast);
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

	map<ContractDefinition const*, eth::Assembly const*> compiledContracts;
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				if (isRequestedContract(*contract))
					compileContract(*contract, compiledContracts);
	this->link();
	m_stackState = CompilationSuccessful;
	return true;
}

void CompilerStack::link()
{
	for (auto& contract: m_contracts)
	{
		contract.second.object.link(m_libraries);
		contract.second.runtimeObject.link(m_libraries);
		contract.second.cloneObject.link(m_libraries);
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

eth::AssemblyItems const* CompilerStack::assemblyItems(string const& _contractName) const
{
	Contract const& currentContract = contract(_contractName);
	return currentContract.compiler ? &contract(_contractName).compiler->assemblyItems() : nullptr;
}

eth::AssemblyItems const* CompilerStack::runtimeAssemblyItems(string const& _contractName) const
{
	Contract const& currentContract = contract(_contractName);
	return currentContract.compiler ? &contract(_contractName).compiler->runtimeAssemblyItems() : nullptr;
}

string const* CompilerStack::sourceMapping(string const& _contractName) const
{
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

eth::LinkerObject const& CompilerStack::object(string const& _contractName) const
{
	return contract(_contractName).object;
}

eth::LinkerObject const& CompilerStack::runtimeObject(string const& _contractName) const
{
	return contract(_contractName).runtimeObject;
}

eth::LinkerObject const& CompilerStack::cloneObject(string const& _contractName) const
{
	return contract(_contractName).cloneObject;
}

/// FIXME: cache this string
string CompilerStack::assemblyString(string const& _contractName, StringMap _sourceCodes) const
{
	Contract const& currentContract = contract(_contractName);
	if (currentContract.compiler)
		return currentContract.compiler->assemblyString(_sourceCodes);
	else
		return string();
}

/// FIXME: cache the JSON
Json::Value CompilerStack::assemblyJSON(string const& _contractName, StringMap _sourceCodes) const
{
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
	return contractABI(contract(_contractName));
}

Json::Value const& CompilerStack::contractABI(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.abi)
		_contract.abi.reset(new Json::Value(ABI::generate(*_contract.contract)));

	return *_contract.abi;
}

Json::Value const& CompilerStack::natspecUser(string const& _contractName) const
{
	return natspecUser(contract(_contractName));
}

Json::Value const& CompilerStack::natspecUser(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.userDocumentation)
		_contract.userDocumentation.reset(new Json::Value(Natspec::userDocumentation(*_contract.contract)));

	return *_contract.userDocumentation;
}

Json::Value const& CompilerStack::natspecDev(string const& _contractName) const
{
	return natspecDev(contract(_contractName));
}

Json::Value const& CompilerStack::natspecDev(Contract const& _contract) const
{
	if (m_stackState < AnalysisSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	solAssert(_contract.contract, "");

	// caches the result
	if (!_contract.devDocumentation)
		_contract.devDocumentation.reset(new Json::Value(Natspec::devDocumentation(*_contract.contract)));

	return *_contract.devDocumentation;
}

Json::Value CompilerStack::methodIdentifiers(string const& _contractName) const
{
	Json::Value methodIdentifiers(Json::objectValue);
	for (auto const& it: contractDefinition(_contractName).interfaceFunctions())
		methodIdentifiers[it.second->externalSignature()] = toHex(it.first.ref());
	return methodIdentifiers;
}

string const& CompilerStack::metadata(string const& _contractName) const
{
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return contract(_contractName).metadata;
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
	if (m_stackState != CompilationSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Compilation was not successful."));

	return *contract(_contractName).contract;
}

size_t CompilerStack::functionEntryPoint(
	std::string const& _contractName,
	FunctionDefinition const& _function
) const
{
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
	tie(startLine, startColumn) = scanner(*_sourceLocation.sourceName).translatePositionToLineColumn(_sourceLocation.start);
	tie(endLine, endColumn) = scanner(*_sourceLocation.sourceName).translatePositionToLineColumn(_sourceLocation.end);

	return make_tuple(++startLine, ++startColumn, ++endLine, ++endColumn);
}

StringMap CompilerStack::loadMissingSources(SourceUnit const& _ast, std::string const& _sourcePath)
{
	StringMap newSources;
	for (auto const& node: _ast.nodes())
		if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
		{
			string importPath = absolutePath(import->path(), _sourcePath);
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
		string context = sanitizePath(redir.context);
		string prefix = sanitizePath(redir.prefix);

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
		bestMatchTarget = sanitizePath(redir.target);
	}
	string path = bestMatchTarget;
	path.append(_path.begin() + longestPrefix, _path.end());
	return path;
}

void CompilerStack::resolveImports()
{
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
		if (!sourcePair.second.isLibrary)
			toposort(&sourcePair.second);

	swap(m_sourceOrder, sourceOrder);
}

string CompilerStack::absolutePath(string const& _path, string const& _reference)
{
	using path = boost::filesystem::path;
	path p(_path);
	// Anything that does not start with `.` is an absolute path.
	if (p.begin() == p.end() || (*p.begin() != "." && *p.begin() != ".."))
		return _path;
	path result(_reference);
	result.remove_filename();
	for (path::iterator it = p.begin(); it != p.end(); ++it)
		if (*it == "..")
			result = result.parent_path();
		else if (*it != ".")
			result /= *it;
	return result.generic_string();
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
	map<ContractDefinition const*, eth::Assembly const*>& _compiledContracts
)
{
	if (
		_compiledContracts.count(&_contract) ||
		!_contract.annotation().unimplementedFunctions.empty() ||
		!_contract.constructorIsPublic()
	)
		return;
	for (auto const* dependency: _contract.annotation().contractDependencies)
		compileContract(*dependency, _compiledContracts);

	Contract& compiledContract = m_contracts.at(_contract.fullyQualifiedName());

	shared_ptr<Compiler> compiler = make_shared<Compiler>(m_evmVersion, m_optimize, m_optimizeRuns);
	compiledContract.compiler = compiler;

	string metadata = createMetadata(compiledContract);
	compiledContract.metadata = metadata;

	// Prepare CBOR metadata for the bytecode
	bytes cborEncodedHash =
		// CBOR-encoding of the key "bzzr0"
		bytes{0x65, 'b', 'z', 'z', 'r', '0'}+
		// CBOR-encoding of the hash
		bytes{0x58, 0x20} + dev::swarmHash(metadata).asBytes();
	bytes cborEncodedMetadata;
	if (onlySafeExperimentalFeaturesActivated(_contract.sourceUnit().annotation().experimentalFeatures))
		cborEncodedMetadata =
			// CBOR-encoding of {"bzzr0": dev::swarmHash(metadata)}
			bytes{0xa1} +
			cborEncodedHash;
	else
		cborEncodedMetadata =
			// CBOR-encoding of {"bzzr0": dev::swarmHash(metadata), "experimental": true}
			bytes{0xa2} +
			cborEncodedHash +
			bytes{0x6c, 'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l', 0xf5};
	solAssert(cborEncodedMetadata.size() <= 0xffff, "Metadata too large");
	// 16-bit big endian length
	cborEncodedMetadata += toCompactBigEndian(cborEncodedMetadata.size(), 2);

	try
	{
		// Run optimiser and compile the contract.
		compiler->compileContract(_contract, _compiledContracts, cborEncodedMetadata);
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

	_compiledContracts[compiledContract.contract] = &compiler->assembly();

	try
	{
		if (!_contract.isLibrary())
		{
			Compiler cloneCompiler(m_evmVersion, m_optimize, m_optimizeRuns);
			cloneCompiler.compileClone(_contract, _compiledContracts);
			compiledContract.cloneObject = cloneCompiler.assembledObject();
		}
	}
	catch (eth::AssemblyException const&)
	{
		// In some cases (if the constructor requests a runtime function), it is not
		// possible to compile the clone.

		// TODO: Report error / warning
	}
}

string const CompilerStack::lastContractName() const
{
	if (m_contracts.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));
	// try to find some user-supplied contract
	string contractName;
	for (auto const& it: m_sources)
		for (ASTPointer<ASTNode> const& node: it.second.ast->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				contractName = contract->fullyQualifiedName();
	return contractName;
}

CompilerStack::Contract const& CompilerStack::contract(string const& _contractName) const
{
	if (m_contracts.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));

	auto it = m_contracts.find(_contractName);
	if (it != m_contracts.end())
		return it->second;

	// To provide a measure of backward-compatibility, if a contract is not located by its
	// fully-qualified name, a lookup will be attempted purely on the contract's name to see
	// if anything will satisfy.
	if (_contractName.find(":") == string::npos)
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
		meta["sources"][s.first]["keccak256"] =
			"0x" + toHex(dev::keccak256(s.second.scanner->source()).asBytes());
		if (m_metadataLiteralSources)
			meta["sources"][s.first]["content"] = s.second.scanner->source();
		else
		{
			meta["sources"][s.first]["urls"] = Json::arrayValue;
			meta["sources"][s.first]["urls"].append(
				"bzzr://" + toHex(dev::swarmHash(s.second.scanner->source()).asBytes())
			);
		}
	}
	meta["settings"]["optimizer"]["enabled"] = m_optimize;
	meta["settings"]["optimizer"]["runs"] = m_optimizeRuns;
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

string CompilerStack::computeSourceMapping(eth::AssemblyItems const& _items) const
{
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
			location.sourceName && sourceIndicesMap.count(*location.sourceName) ?
			sourceIndicesMap.at(*location.sourceName) :
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
				ret += std::to_string(location.start);
			if (components-- > 0)
			{
				ret += ':';
				if (length != prevLength)
					ret += std::to_string(length);
				if (components-- > 0)
				{
					ret += ':';
					if (sourceIndex != prevSourceIndex)
						ret += std::to_string(sourceIndex);
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
	if (!assemblyItems(_contractName) && !runtimeAssemblyItems(_contractName))
		return Json::Value();

	using Gas = GasEstimator::GasConsumption;
	GasEstimator gasEstimator(m_evmVersion);
	Json::Value output(Json::objectValue);

	if (eth::AssemblyItems const* items = assemblyItems(_contractName))
	{
		Gas executionGas = gasEstimator.functionalEstimation(*items);
		u256 bytecodeSize(runtimeObject(_contractName).bytecode.size());
		Gas codeDepositGas = bytecodeSize * eth::GasCosts::createDataGas;

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
