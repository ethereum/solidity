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
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/parsing/Parser.h>
#include <libsolidity/analysis/GlobalContext.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolidity/analysis/DocStringAnalyser.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/InterfaceHandler.h>
#include <libsolidity/formal/Why3Translator.h>

#include <libdevcore/SHA3.h>

using namespace std;

namespace dev
{
namespace solidity
{

const map<string, string> StandardSources = map<string, string>{
	{"coin", R"(import "CoinReg";import "Config";import "configUser";contract coin is configUser{function coin(bytes3 name, uint denom) {CoinReg(Config(configAddr()).lookup(3)).register(name, denom);}})"},
	{"Coin", R"(contract Coin{function isApprovedFor(address _target,address _proxy)constant returns(bool _r){}function isApproved(address _proxy)constant returns(bool _r){}function sendCoinFrom(address _from,uint256 _val,address _to){}function coinBalanceOf(address _a)constant returns(uint256 _r){}function sendCoin(uint256 _val,address _to){}function coinBalance()constant returns(uint256 _r){}function approve(address _a){}})"},
	{"CoinReg", R"(contract CoinReg{function count()constant returns(uint256 r){}function info(uint256 i)constant returns(address addr,bytes3 name,uint256 denom){}function register(bytes3 name,uint256 denom){}function unregister(){}})"},
	{"configUser", R"(contract configUser{function configAddr()constant returns(address a){ return 0xc6d9d2cd449a754c494264e1809c50e34d64562b;}})"},
	{"Config", R"(contract Config{function lookup(uint256 service)constant returns(address a){}function kill(){}function unregister(uint256 id){}function register(uint256 id,address service){}})"},
	{"mortal", R"(import "owned";contract mortal is owned {function kill() { if (msg.sender == owner) suicide(owner); }})"},
	{"named", R"(import "Config";import "NameReg";import "configUser";contract named is configUser {function named(bytes32 name) {NameReg(Config(configAddr()).lookup(1)).register(name);}})"},
	{"NameReg", R"(contract NameReg{function register(bytes32 name){}function addressOf(bytes32 name)constant returns(address addr){}function unregister(){}function nameOf(address addr)constant returns(bytes32 name){}})"},
	{"owned", R"(contract owned{function owned(){owner = msg.sender;}modifier onlyowner(){if(msg.sender==owner)_}address owner;})"},
	{"service", R"(import "Config";import "configUser";contract service is configUser{function service(uint _n){Config(configAddr()).register(_n, this);}})"},
	{"std", R"(import "owned";import "mortal";import "Config";import "configUser";import "NameReg";import "named";)"}
};

CompilerStack::CompilerStack(bool _addStandardSources):
	m_parseSuccessful(false)
{
	if (_addStandardSources)
		addSources(StandardSources, true); // add them as libraries
}

void CompilerStack::reset(bool _keepSources, bool _addStandardSources)
{
	m_parseSuccessful = false;
	if (_keepSources)
		for (auto sourcePair: m_sources)
			sourcePair.second.reset();
	else
	{
		m_sources.clear();
		if (_addStandardSources)
			addSources(StandardSources, true);
	}
	m_globalContext.reset();
	m_sourceOrder.clear();
	m_contracts.clear();
	m_errors.clear();
}

bool CompilerStack::addSource(string const& _name, string const& _content, bool _isLibrary)
{
	bool existed = m_sources.count(_name) != 0;
	reset(true);
	m_sources[_name].scanner = make_shared<Scanner>(CharStream(_content), _name);
	m_sources[_name].isLibrary = _isLibrary;
	return existed;
}

void CompilerStack::setSource(string const& _sourceCode)
{
	reset();
	addSource("", _sourceCode);
}

bool CompilerStack::parse()
{
	//reset
	m_errors.clear();
	m_parseSuccessful = false;

	map<string, SourceUnit const*> sourceUnitsByName;
	for (auto& sourcePair: m_sources)
	{
		sourcePair.second.scanner->reset();
		sourcePair.second.ast = Parser(m_errors).parse(sourcePair.second.scanner);
		if (!sourcePair.second.ast)
			solAssert(!Error::containsOnlyWarnings(m_errors), "Parser returned null but did not report error.");
		else
			sourcePair.second.ast->annotation().path = sourcePair.first;
		sourceUnitsByName[sourcePair.first] = sourcePair.second.ast.get();
	}
	if (!Error::containsOnlyWarnings(m_errors))
		// errors while parsing. sould stop before type checking
		return false;

	resolveImports();

	bool noErrors = true;
	DocStringAnalyser docStringAnalyser(m_errors);
	for (Source const* source: m_sourceOrder)
		if (!docStringAnalyser.analyseDocStrings(*source->ast))
			noErrors = false;

	m_globalContext = make_shared<GlobalContext>();
	NameAndTypeResolver resolver(m_globalContext->declarations(), m_errors);
	for (Source const* source: m_sourceOrder)
		if (!resolver.registerDeclarations(*source->ast))
			return false;

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
				m_contracts[contract->name()].contract = contract;
			}

	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->nodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				m_globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*m_globalContext->currentThis());
				TypeChecker typeChecker(m_errors);
				if (typeChecker.checkTypeRequirements(*contract))
				{
					contract->setDevDocumentation(InterfaceHandler::devDocumentation(*contract));
					contract->setUserDocumentation(InterfaceHandler::userDocumentation(*contract));
				}
				else
					noErrors = false;

				m_contracts[contract->name()].contract = contract;
			}
	m_parseSuccessful = noErrors;
	return m_parseSuccessful;
}

bool CompilerStack::parse(string const& _sourceCode)
{
	setSource(_sourceCode);
	return parse();
}

vector<string> CompilerStack::contractNames() const
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	vector<string> contractNames;
	for (auto const& contract: m_contracts)
		contractNames.push_back(contract.first);
	return contractNames;
}


bool CompilerStack::compile(bool _optimize, unsigned _runs)
{
	if (!m_parseSuccessful)
		if (!parse())
			return false;

	map<ContractDefinition const*, eth::Assembly const*> compiledContracts;
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->nodes())
			if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
				compileContract(_optimize, _runs, *contract, compiledContracts);
	return true;
}

bool CompilerStack::compile(string const& _sourceCode, bool _optimize)
{
	return parse(_sourceCode) && compile(_optimize);
}

void CompilerStack::link(const std::map<string, h160>& _libraries)
{
	for (auto& contract: m_contracts)
	{
		contract.second.object.link(_libraries);
		contract.second.runtimeObject.link(_libraries);
		contract.second.cloneObject.link(_libraries);
	}
}

bool CompilerStack::prepareFormalAnalysis()
{
	Why3Translator translator(m_errors);
	for (Source const* source: m_sourceOrder)
		if (!translator.process(*source->ast))
			return false;

	m_formalTranslation = translator.translation();

	return true;
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

dev::h256 CompilerStack::contractCodeHash(string const& _contractName) const
{
	auto const& obj = runtimeObject(_contractName);
	if (obj.bytecode.empty() || !obj.linkReferences.empty())
		return dev::h256();
	else
		return dev::sha3(obj.bytecode);
}

Json::Value CompilerStack::streamAssembly(ostream& _outStream, string const& _contractName, StringMap _sourceCodes, bool _inJsonFormat) const
{
	Contract const& currentContract = contract(_contractName);
	if (currentContract.compiler)
		return currentContract.compiler->streamAssembly(_outStream, _sourceCodes, _inJsonFormat);
	else
	{
		_outStream << "Contract not fully implemented" << endl;
		return Json::Value();
	}
}

string const& CompilerStack::interface(string const& _contractName) const
{
	return metadata(_contractName, DocumentationType::ABIInterface);
}

string const& CompilerStack::solidityInterface(string const& _contractName) const
{
	return metadata(_contractName, DocumentationType::ABISolidityInterface);
}

string const& CompilerStack::metadata(string const& _contractName, DocumentationType _type) const
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	std::unique_ptr<string const>* doc;
	Contract const& currentContract = contract(_contractName);

	// checks wheather we already have the documentation
	switch (_type)
	{
	case DocumentationType::NatspecUser:
		doc = &currentContract.userDocumentation;
		break;
	case DocumentationType::NatspecDev:
		doc = &currentContract.devDocumentation;
		break;
	case DocumentationType::ABIInterface:
		doc = &currentContract.interface;
		break;
	case DocumentationType::ABISolidityInterface:
		doc = &currentContract.solidityInterface;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Illegal documentation type."));
	}

	// caches the result
	if (!*doc)
		doc->reset(new string(InterfaceHandler::documentation(*currentContract.contract, _type)));

	return *(*doc);
}

Scanner const& CompilerStack::scanner(string const& _sourceName) const
{
	return *source(_sourceName).scanner;
}

SourceUnit const& CompilerStack::ast(string const& _sourceName) const
{
	return *source(_sourceName).ast;
}

ContractDefinition const& CompilerStack::contractDefinition(string const& _contractName) const
{
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

void CompilerStack::resolveImports()
{
	// topological sorting (depth first search) of the import graph, cutting potential cycles
	vector<Source const*> sourceOrder;
	set<Source const*> sourcesSeen;

	function<void(string const&, Source const*)> toposort = [&](string const& _sourceName, Source const* _source)
	{
		if (sourcesSeen.count(_source))
			return;
		sourcesSeen.insert(_source);
		for (ASTPointer<ASTNode> const& node: _source->ast->nodes())
			if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
			{
				string path = absolutePath(import->path(), _sourceName);
				import->annotation().absolutePath = path;
				if (!m_sources.count(path))
					BOOST_THROW_EXCEPTION(
						Error(Error::Type::ParserError)
							<< errinfo_sourceLocation(import->location())
							<< errinfo_comment("Source not found.")
					);
				import->annotation().sourceUnit = m_sources.at(path).ast.get();

				toposort(path, &m_sources[path]);
			}
		sourceOrder.push_back(_source);
	};

	for (auto const& sourcePair: m_sources)
		if (!sourcePair.second.isLibrary)
			toposort(sourcePair.first, &sourcePair.second);

	swap(m_sourceOrder, sourceOrder);
}

string CompilerStack::absolutePath(string const& _path, string const& _reference) const
{
	// Anything that does not start with `.` is an absolute path.
	if (_path.empty() || _path.front() != '.')
		return _path;
	using path = boost::filesystem::path;
	path p(_path);
	path result(_reference);
	result.remove_filename();
	for (path::iterator it = p.begin(); it != p.end(); ++it)
		if (*it == "..")
			result = result.parent_path();
		else if (*it != ".")
			result /= *it;
	return result.string();
}

void CompilerStack::compileContract(
	bool _optimize,
	unsigned _runs,
	ContractDefinition const& _contract,
	map<ContractDefinition const*, eth::Assembly const*>& _compiledContracts
)
{
	if (_compiledContracts.count(&_contract) || !_contract.annotation().isFullyImplemented)
		return;
	for (auto const* dependency: _contract.annotation().contractDependencies)
		compileContract(_optimize, _runs, *dependency, _compiledContracts);

	shared_ptr<Compiler> compiler = make_shared<Compiler>(_optimize, _runs);
	compiler->compileContract(_contract, _compiledContracts);
	Contract& compiledContract = m_contracts.at(_contract.name());
	compiledContract.compiler = compiler;
	compiledContract.object = compiler->assembledObject();
	compiledContract.runtimeObject = compiler->runtimeObject();
	_compiledContracts[compiledContract.contract] = &compiler->assembly();

	Compiler cloneCompiler(_optimize, _runs);
	cloneCompiler.compileClone(_contract, _compiledContracts);
	compiledContract.cloneObject = cloneCompiler.assembledObject();
}

std::string CompilerStack::defaultContractName() const
{
	return contract("").contract->name();
}

CompilerStack::Contract const& CompilerStack::contract(string const& _contractName) const
{
	if (m_contracts.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));
	string contractName = _contractName;
	if (_contractName.empty())
		// try to find some user-supplied contract
		for (auto const& it: m_sources)
			if (!StandardSources.count(it.first))
				for (ASTPointer<ASTNode> const& node: it.second.ast->nodes())
					if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
						contractName = contract->name();
	auto it = m_contracts.find(contractName);
	if (it == m_contracts.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Contract " + _contractName + " not found."));
	return it->second;
}

CompilerStack::Source const& CompilerStack::source(string const& _sourceName) const
{
	auto it = m_sources.find(_sourceName);
	if (it == m_sources.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Given source file not found."));

	return it->second;
}

}
}
