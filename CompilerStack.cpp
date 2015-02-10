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
#include <libsolidity/AST.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/GlobalContext.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/InterfaceHandler.h>

#include <libdevcrypto/SHA3.h>

using namespace std;

namespace dev
{
namespace solidity
{

const map<string, string> StandardSources = map<string, string>{
	{"coin", R"(import "CoinReg";import "Config";import "configUser";contract coin is configUser{function coin(string3 name, uint denom) {CoinReg(Config(configAddr()).lookup(3)).register(name, denom);}})"},
	{"Coin", R"(contract Coin{function isApprovedFor(address _target,address _proxy)constant returns(bool _r){}function isApproved(address _proxy)constant returns(bool _r){}function sendCoinFrom(address _from,uint256 _val,address _to){}function coinBalanceOf(address _a)constant returns(uint256 _r){}function sendCoin(uint256 _val,address _to){}function coinBalance()constant returns(uint256 _r){}function approve(address _a){}})"},
	{"CoinReg", R"(contract CoinReg{function count()constant returns(uint256 r){}function info(uint256 i)constant returns(address addr,string3 name,uint256 denom){}function register(string3 name,uint256 denom){}function unregister(){}})"},
	{"configUser", R"(contract configUser{function configAddr()constant returns(address a){ return 0xc6d9d2cd449a754c494264e1809c50e34d64562b;}})"},
	{"Config", R"(contract Config{function lookup(uint256 service)constant returns(address a){}function kill(){}function unregister(uint256 id){}function register(uint256 id,address service){}})"},
	{"mortal", R"(import "owned";contract mortal is owned {function kill() { if (msg.sender == owner) suicide(owner); }})"},
	{"named", R"(import "Config";import "NameReg";import "configUser";contract named is configUser {function named(string32 name) {NameReg(Config(configAddr()).lookup(1)).register(name);}})"},
	{"NameReg", R"(contract NameReg{function register(string32 name){}function addressOf(string32 name)constant returns(address addr){}function unregister(){}function nameOf(address addr)constant returns(string32 name){}})"},
	{"owned", R"(contract owned{function owned(){owner = msg.sender;}modifier onlyowner(){if(msg.sender==owner)_}address owner;})"},
	{"service", R"(import "Config";import "configUser";contract service is configUser{function service(uint _n){Config(configAddr()).register(_n, this);}})"},
	{"std", R"(import "owned";import "mortal";import "Config";import "configUser";import "NameReg";import "named";)"}
};

CompilerStack::CompilerStack(bool _addStandardSources):
	m_addStandardSources(_addStandardSources), m_parseSuccessful(false)
{
	if (m_addStandardSources)
		addSources(StandardSources);
}

bool CompilerStack::addSource(string const& _name, string const& _content)
{
	bool existed = m_sources.count(_name) != 0;
	reset(true);
	m_sources[_name].scanner = make_shared<Scanner>(CharStream(expanded(_content)), _name);
	return existed;
}

void CompilerStack::setSource(string const& _sourceCode)
{
	reset();
	addSource("", expanded(_sourceCode));
}

void CompilerStack::parse()
{
	for (auto& sourcePair: m_sources)
	{
		sourcePair.second.scanner->reset();
		sourcePair.second.ast = Parser().parse(sourcePair.second.scanner);
	}
	resolveImports();

	m_globalContext = make_shared<GlobalContext>();
	NameAndTypeResolver resolver(m_globalContext->getDeclarations());
	for (Source const* source: m_sourceOrder)
		resolver.registerDeclarations(*source->ast);
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				m_globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*m_globalContext->getCurrentThis());
				resolver.updateDeclaration(*m_globalContext->getCurrentSuper());
				resolver.resolveNamesAndTypes(*contract);
				m_contracts[contract->getName()].contract = contract;
			}
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				m_globalContext->setCurrentContract(*contract);
				resolver.updateDeclaration(*m_globalContext->getCurrentThis());
				resolver.checkTypeRequirements(*contract);
				m_contracts[contract->getName()].contract = contract;
			}
	m_parseSuccessful = true;
}

void CompilerStack::parse(string const& _sourceCode)
{
	setSource(_sourceCode);
	parse();
}

vector<string> CompilerStack::getContractNames() const
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));
	vector<string> contractNames;
	for (auto const& contract: m_contracts)
		contractNames.push_back(contract.first);
	return contractNames;
}

////// BEGIN: TEMPORARY ONLY
///
/// NOTE: THIS INVALIDATES SOURCE POINTERS AND CAN CRASH THE COMPILER
///
/// remove once import works properly and we have genesis contracts

string CompilerStack::expanded(string const& _sourceCode)
{
	const map<string, string> c_standardSources = map<string, string>{
		{ "Config", "contract Config{function lookup(uint256 service)constant returns(address a){}function kill(){}function unregister(uint256 id){}function register(uint256 id,address service){}}" },
		{ "Coin", "contract Coin{function isApprovedFor(address _target,address _proxy)constant returns(bool _r){}function isApproved(address _proxy)constant returns(bool _r){}function sendCoinFrom(address _from,uint256 _val,address _to){}function coinBalanceOf(address _a)constant returns(uint256 _r){}function sendCoin(uint256 _val,address _to){}function coinBalance()constant returns(uint256 _r){}function approve(address _a){}}"},
		{ "CoinReg", "contract CoinReg{function count()constant returns(uint256 r){}function info(uint256 i)constant returns(address addr,string3 name,uint256 denom){}function register(string3 name,uint256 denom){}function unregister(){}}" },
		{ "coin", "#require CoinReg\ncontract coin {function coin(string3 name, uint denom) {CoinReg(Config().lookup(3)).register(name, denom);}}" },
		{ "service", "#require Config\ncontract service{function service(uint _n){Config().register(_n, this);}}" },
		{ "owned", "contract owned{function owned(){owner = msg.sender;}modifier onlyowner(){if(msg.sender==owner)_}address owner;}" },
		{ "mortal", "#require owned\ncontract mortal is owned {function kill() { if (msg.sender == owner) suicide(owner); }}" },
		{ "NameReg", "contract NameReg{function register(string32 name){}function addressOf(string32 name)constant returns(address addr){}function unregister(){}function nameOf(address addr)constant returns(string32 name){}}" },
		{ "named", "#require Config NameReg\ncontract named {function named(string32 name) {NameReg(Config().lookup(1)).register(name);}}" },
		{ "std", "#require owned mortal Config NameReg named" },
	};

	string sub;
	set<string> got;
	function<string(string const&)> localExpanded;
	localExpanded = [&](string const& s) -> string
	{
		string ret = s;
		for (size_t p = 0; p != string::npos;)
			if ((p = ret.find("#require ")) != string::npos)
			{
				string n = ret.substr(p + 9, ret.find_first_of('\n', p + 9) - p - 9);
				ret.replace(p, n.size() + 9, "");
				vector<string> rs;
				boost::split(rs, n, boost::is_any_of(" \t,"), boost::token_compress_on);
				for (auto const& r: rs)
					if (!got.count(r))
					{
						if (c_standardSources.count(r))
							sub.append("\n" + localExpanded(c_standardSources.at(r)) + "\n");
						got.insert(r);
					}
			}
			// TODO: remove once we have genesis contracts.
			else if ((p = ret.find("Config()")) != string::npos)
				ret.replace(p, 8, "Config(0xc6d9d2cd449a754c494264e1809c50e34d64562b)");
		return ret;
	};
	return sub + localExpanded(_sourceCode);
}

////// END: TEMPORARY ONLY

void CompilerStack::compile(bool _optimize)
{
	if (!m_parseSuccessful)
		parse();

	map<ContractDefinition const*, bytes const*> contractBytecode;
	for (Source const* source: m_sourceOrder)
		for (ASTPointer<ASTNode> const& node: source->ast->getNodes())
			if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			{
				shared_ptr<Compiler> compiler = make_shared<Compiler>(_optimize);
				compiler->compileContract(*contract, contractBytecode);
				Contract& compiledContract = m_contracts[contract->getName()];
				compiledContract.bytecode = compiler->getAssembledBytecode();
				compiledContract.runtimeBytecode = compiler->getRuntimeBytecode();
				compiledContract.compiler = move(compiler);
				contractBytecode[compiledContract.contract] = &compiledContract.bytecode;
			}
}

bytes const& CompilerStack::compile(string const& _sourceCode, bool _optimize)
{
	parse(_sourceCode);
	compile(_optimize);
	return getBytecode();
}

bytes const& CompilerStack::getBytecode(string const& _contractName) const
{
	return getContract(_contractName).bytecode;
}

bytes const& CompilerStack::getRuntimeBytecode(string const& _contractName) const
{
	return getContract(_contractName).runtimeBytecode;
}

dev::h256 CompilerStack::getContractCodeHash(string const& _contractName) const
{
	return dev::sha3(getRuntimeBytecode(_contractName));
}

void CompilerStack::streamAssembly(ostream& _outStream, string const& _contractName) const
{
	getContract(_contractName).compiler->streamAssembly(_outStream);
}

string const& CompilerStack::getInterface(string const& _contractName) const
{
	return getMetadata(_contractName, DocumentationType::ABIInterface);
}

string const& CompilerStack::getSolidityInterface(string const& _contractName) const
{
	return getMetadata(_contractName, DocumentationType::ABISolidityInterface);
}

string const& CompilerStack::getMetadata(string const& _contractName, DocumentationType _type) const
{
	if (!m_parseSuccessful)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Parsing was not successful."));

	Contract const& contract = getContract(_contractName);

	std::unique_ptr<string const>* doc;
	switch (_type)
	{
	case DocumentationType::NatspecUser:
		doc = &contract.userDocumentation;
		break;
	case DocumentationType::NatspecDev:
		doc = &contract.devDocumentation;
		break;
	case DocumentationType::ABIInterface:
		doc = &contract.interface;
		break;
	case DocumentationType::ABISolidityInterface:
		doc = &contract.solidityInterface;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Illegal documentation type."));
	}
	if (!*doc)
		*doc = contract.interfaceHandler->getDocumentation(*contract.contract, _type);
	return *(*doc);
}

Scanner const& CompilerStack::getScanner(string const& _sourceName) const
{
	return *getSource(_sourceName).scanner;
}

SourceUnit const& CompilerStack::getAST(string const& _sourceName) const
{
	return *getSource(_sourceName).ast;
}

ContractDefinition const& CompilerStack::getContractDefinition(string const& _contractName) const
{
	return *getContract(_contractName).contract;
}

bytes CompilerStack::staticCompile(std::string const& _sourceCode, bool _optimize)
{
	CompilerStack stack;
	return stack.compile(_sourceCode, _optimize);
}

void CompilerStack::reset(bool _keepSources)
{
	m_parseSuccessful = false;
	if (_keepSources)
		for (auto sourcePair: m_sources)
			sourcePair.second.reset();
	else
	{
		m_sources.clear();
		if (m_addStandardSources)
			addSources(StandardSources);
	}
	m_globalContext.reset();
	m_sourceOrder.clear();
	m_contracts.clear();
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
		for (ASTPointer<ASTNode> const& node: _source->ast->getNodes())
			if (ImportDirective const* import = dynamic_cast<ImportDirective*>(node.get()))
			{
				string const& id = import->getIdentifier();
				if (!m_sources.count(id))
					BOOST_THROW_EXCEPTION(ParserError()
										  << errinfo_sourceLocation(import->getLocation())
										  << errinfo_comment("Source not found."));
				toposort(&m_sources[id]);
			}
		sourceOrder.push_back(_source);
	};

	for (auto const& sourcePair: m_sources)
		toposort(&sourcePair.second);

	swap(m_sourceOrder, sourceOrder);
}

CompilerStack::Contract const& CompilerStack::getContract(string const& _contractName) const
{
	if (m_contracts.empty())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("No compiled contracts found."));
	string contractName = _contractName;
	if (_contractName.empty())
		// try to find some user-supplied contract
		for (auto const& it: m_sources)
			if (!StandardSources.count(it.first))
				for (ASTPointer<ASTNode> const& node: it.second.ast->getNodes())
					if (auto contract = dynamic_cast<ContractDefinition const*>(node.get()))
						contractName = contract->getName();
	auto it = m_contracts.find(contractName);
	if (it == m_contracts.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Contract " + _contractName + " not found."));
	return it->second;
}

CompilerStack::Source const& CompilerStack::getSource(string const& _sourceName) const
{
	auto it = m_sources.find(_sourceName);
	if (it == m_sources.end())
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("Given source file not found."));
	return it->second;
}

CompilerStack::Contract::Contract(): interfaceHandler(make_shared<InterfaceHandler>()) {}

}
}
