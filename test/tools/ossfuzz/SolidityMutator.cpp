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

#include <libsolutil/Common.h>

#include <test/tools/ossfuzz/SolidityMutator.h>

#include <boost/algorithm/string/join.hpp>

#include <algorithm>
#include <regex>

using namespace std;
using namespace solidity::util;
using namespace solidity::test::fuzzer;
using SolP = solidity::test::fuzzer::SolidityParser;

string Generator::operator()(std::string&& _template, std::map<std::string, std::string>&& _args)
{
	Whiskers w(move(_template));
	for (auto &&item: _args)
		w(move(item.first), move(item.second));
	return w.render();
}

template <typename T>
void SolidityMutator::fuzzRule(T* _ctx)
{
	mutate(_ctx);
//	if (coinToss())
//		gen();
}

void SolidityMutator::gen()
{
	static const vector<string> validPragmas = {
		"experimental SMTChecker",
		"experimental ABIEncoderV2"
	};
	m_out << generator(
		[&]() {
			return Whiskers(R"(pragma <string>;)")
				("string", validPragmas[m_rand() % validPragmas.size()])
				.render();
		},
		randOneToN(s_maxPragmas),
		"\n",
		true
	);
}

void SolidityMutator::mutate(SolP::PragmaDirectiveContext*)
{
	m_out << Whiskers(R"(pragma <mutation>;)")
		("mutation", mutatePragma())
		.render();
}

string SolidityMutator::eventParam()
{
	return Whiskers(R"(<typename> <indexed> <id>)")
		("typename", typeName())
		("indexed", coinToss() ? "indexed" : "")
		("id", coinToss() ? genRandomId("ev"): "")
		.render();
}

string SolidityMutator::visibility()
{
	if (interfaceScope())
		return "external";

	switch (randOneToN(4))
	{
	case 1:
		return "public";
	case 2:
		return "external";
	case 3:
		return "private";
	case 4:
		return "internal";
	}
	assert(false);
}

string SolidityMutator::mutability()
{
	switch (randOneToN(4))
	{
	case 1:
		return "view";
	case 2:
		return "pure";
	case 3:
		if (libraryScope() || globalScope())
			return "";
		else
			return "payable";
	case 4:
		return "";
	}
	assert(false);
}

string SolidityMutator::elementaryType(bool _allowAddressPayable)
{
	switch (randOneToN(8))
	{
	case 1:
		return "address";
	case 2:
		if (_allowAddressPayable)
			return "address payable";
		else
			return "address";
	case 3:
		return "bool";
	case 4:
		return "string";
	case 5:
		return "bytes";
	case 6:
		return "int" + bitWidth();
	case 7:
		return "uint" + bitWidth();
	case 8:
		return "bytes" + byteWidth();
	}
	assert(false);
}

string SolidityMutator::typeName()
{
	switch (randOneToN(5))
	{
	case 1:
		// TODO: Implement function type mutator
		return "function () external view returns (uint)";
	case 2:
		return Whiskers(R"(mapping (<keyType> => <typeName>))")
			("keyType", elementaryType(false))
			("typeName", typeName())
			.render();
	case 3:
		if (m_numStructs > 0)
			return "s" + to_string(m_rand() % m_numStructs);
		else
			return elementaryType();
	case 4:
		// TODO: Implement complex types
		return "int[]";
	case 5:
		return elementaryType();
	}
	assert(false);
}

string SolidityMutator::dataLocation()
{
	switch (randOneToN(3))
	{
	case 1:
		return "storage";
	case 2:
		return "memory";
	case 3:
		return "calldata";
	}
	assert(false);
}

template <class C>
antlrcpp::Any SolidityMutator::genericVisitor(C* _ctx, std::string _ctxName)
{
	visitChildren(_ctx);
	return antlrcpp::Any();
}

string SolidityMutator::genRandomUserDefinedTypeName(unsigned _numIds, unsigned _len, char _start, char _end)
{
	uniform_int_distribution<char> dist(_start, _end);
	vector<string> ids{};
	generate_n(back_inserter(ids), m_rand() % _numIds + 1, [&](){ return genRandString(_len, _start, _end); });
	return boost::algorithm::join(ids, ".");
}

string SolidityMutator::genRandString(unsigned int _maxLen, char _start, char _end)
{
	uniform_int_distribution<char> dist(_start, _end);
	string result{};
	generate_n(back_inserter(result), m_rand() % _maxLen + 1, [&]{ return dist(m_rand); });
	return result;
}

template <typename T, typename F, typename... Args>
void SolidityMutator::visitItemsOrGenerate(T _container, F&& _generator, Args&&... _args)
{
	if (_container.size() == 0)
		mayRun(forward<F>(_generator), forward<Args>(_args)...);
	else
		for (auto item: _container)
			item->accept(this);
}

template <typename F, typename... Args>
auto SolidityMutator::mayRun(F&& _func, Args&&... _args, unsigned _n)
{
	if (m_rand() % _n == 0)
		return invoke(forward<F>(_func), forward<Args>(_args)...);
	else
		return result_of<F>::type();
}

antlrcpp::Any SolidityMutator::visitSourceUnit(SolP::SourceUnitContext* _ctx)
{
//	for (auto &item: _ctx->getRuleContexts<SolP::SourceUnitContext>())
//		visitItemsOrGenerate(item, gen, this, item);

//	mayRun(
//		[](SolP::SourceUnitContext* _ctx) {
//			visitItemsOrGenerate(
//				_ctx->pragmaDirective(),
//				[&]() {	m_out << "pragma solidity >= 0.0.0;"; }
//			);
//			visitItemsOrGenerate(
//				_ctx->importDirective(),
//				[]() {}
//			);
//			visitItemsOrGenerate(
//				_ctx->contractDefinition(),
//				[&]() { genContract(); }
//			);
//			visitItemsOrGenerate(
//				_ctx->interfaceDefinition(),
//				[&]() {
//					m_out << Whiskers(R"(interface <id> { function f0() external; })")
//						("id", "I" + to_string(m_numInterfaces++))
//						.render();
//				}
//			);
//			visitItemsOrGenerate(
//			  _ctx->libraryDefinition(),
//			  [&]() {
//				m_out << Whiskers(R"(library <id> { function f0() public {} })")
//					("id", "L" + to_string(m_numLibraries++))
//					.render();
//			  }
//			);
//			visitItemsOrGenerate(
//			  _ctx->functionDefinition(),
//			  [&]() {
//				m_out << "function func() {}";
//			  }
//			);
//			visitItemsOrGenerate(
//			  _ctx->structDefinition(),
//			  [&]() {
//				m_out << "struct S { uint sm0; }";
//			  }
//			);
//			visitItemsOrGenerate(
//			  _ctx->enumDefinition(),
//			  [&]() {
//				m_out << "enum E { first }";
//			  }
//			);
//		}
//	);


	return antlrcpp::Any();
}

//void SolidityMutator::gen(SolP::ModifierDefinitionContext*)
//{
//	constexpr string s = R"(modifier <id> (<params>) <virtual> <override> <?noDef>;</noDef>)";
//	Whiskers mod(R"(modifier <id> (<params>) <virtual> <override> <?noDef>;</noDef>)");
//}
//
//void SolidityMutator::mutate(SolP::ModifierDefinitionContext* _ctx)
//{
//	for (auto &rule: _ctx->getRuleContexts<SolP::ModifierDefinitionContext>())
//		std::cout << rule->getText() << std::endl;
//}

antlrcpp::Any SolidityMutator::visitModifierDefinition(SolidityParser::ModifierDefinitionContext* _ctx)
{
	Whiskers mod(R"(modifier <id> (<params>) <virtual> <override> <?noDef>;</noDef>)");

	bool noDef = coinToss();
	m_out << mod
		("id", genRandomId("m", 2))
		("params", _ctx->arguments ? _ctx->arguments->accept(this).as<string>() : "")
		("virtual", coinToss() ? "virtual" : "")
		("override", coinToss() ? "override" : "")
		("noDef", noDef)
		.render();
	if (!noDef && _ctx->body)
		_ctx->body->accept(this);
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitStructDefinition(SolP::StructDefinitionContext* _ctx)
{
	Whiskers structDef(R"(struct <id> { <members> })");
	unsigned numMembers = 0;
	auto fieldGenerator = [&]() {
		return Whiskers(R"(<typeName> <id>;)")
			("typeName", typeName())
			("id", "sm" + to_string(numMembers++))
			.render();
	};
	structDef("members", generator(fieldGenerator, randOneToN(5), " ", true));
	structDef("id", "s" + to_string(m_numStructs++));
	m_out << structDef.render();
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitEnumDefinition(SolP::EnumDefinitionContext* _ctx)
{
	unsigned numEnums = 0;
	string elements = generator(
		[&]() { return "e" + to_string(numEnums++); },
		randOneToN(5),
		", ",
		false
	);
	m_out << Whiskers(R"(enum <id> {<elements>})")
		("id", "en" + to_string(m_numEnums++))
		("elements", elements)
		.render();
	return antlrcpp::Any();
}

string SolidityMutator::generator(function<string()> _gen, unsigned _n, string _separator, bool _removeDups)
{
	vector<string> g{};
	generate_n(
		back_inserter(g),
		_n,
		_gen
	);
	if (_removeDups)
	{
		sort(g.begin(), g.end());
		g.erase(unique(g.begin(), g.end()), g.end());
	}
	return boost::algorithm::join(g, _separator);
}

string SolidityMutator::mutatePragma()
{
	string minVersion = generator(
		[&]() { return genRandString(1, '0', '9'); },
		3,
		".",
		false
	);
	string maxVersion = generator(
		[&]() { return genRandString(1, '0', '9'); },
		3,
		".",
		false
	);
	string lSign = coinToss() ? ">" : ">=";
	string uSign = coinToss() ? "<" : "<=";
	return Whiskers(R"(solidity <?l><lSign> <minV></l><?u> <uSign> <maxV></u>)")
		("l", coinToss())
		("u", coinToss())
		("lSign", lSign)
		("minV", minVersion)
		("uSign", uSign)
		("maxV", maxVersion)
		.render();
}

void SolidityMutator::genContract()
{
	m_out << Whiskers(R"(contract <id> { function f0() public {} })")
		("id", "C" + to_string(m_numContracts++))
		.render();
}

antlrcpp::Any SolidityMutator::visitPragmaDirective(SolP::PragmaDirectiveContext* _ctx)
{
	fuzzRule(_ctx);
	return antlrcpp::Any();
}

string SolidityMutator::genImport()
{
	string path = genRandomPath();
	string id = genRandomId("Im");

	Whiskers importPathAsId(R"(import <path> as <id>;)");
	Whiskers importStarAsIdFromPath(R"(import * as <id> from <path>;)");
	Whiskers importPath(R"(import <path>;)");

	switch (randOneToN(4))
	{
	case 1:
		return importPathAsId("path", path)("id", id).render();
	case 2:
		return importStarAsIdFromPath("id", id)("path", path).render();
	case 3:
	{
		vector<string> symbols{};
		unsigned numElements = randOneToN(10);
		generate_n(
			back_inserter(symbols),
			numElements,
			[&](){
			  return Whiskers((R"(<?as><path> as <id><!as><symbol></as>)"))
				  ("as", coinToss())
				  ("path", genRandomPath())
				  ("id", genRandomId("Im"))
				  ("symbol", genRandString(5, 'A', 'z'))
				  .render();
			}
		);
		return Whiskers(R"(import {<syms>} from <path>;)")
			("syms", boost::algorithm::join(symbols, ", "))
			("path", path)
			.render();
	}
	case 4:
		return importPath("path", path).render();
	}
	assert(false);
}

antlrcpp::Any SolidityMutator::visitImportDirective(SolP::ImportDirectiveContext* _ctx)
{
	m_out << regex_replace(
		_ctx->getText(),
		regex("as|from|*|"),
		" $& "
	);
	return antlrcpp::Any();
}

template<typename T>
void SolidityMutator::interfaceContractLibraryVisitor(T* _ctx)
{
	assert(
		antlrcpp::is<SolP::ContractDefinitionContext*>(_ctx) ||
	    antlrcpp::is<SolP::InterfaceDefinitionContext*>(_ctx) ||
        antlrcpp::is<SolP::LibraryDefinitionContext*>(_ctx)
    );

	string baseNames{};
	bool atLeastOneBaseContract = m_numContracts > 0;
	bool atLeastOneBaseInterface = m_numInterfaces > 0;
	if (atLeastOneBaseContract || atLeastOneBaseInterface)
		baseNames = generator(
			[&]() {
			  if (contractScope())
			  {
				  if (atLeastOneBaseContract && coinToss())
					  return "C" + to_string(m_rand() % m_numContracts);
				  else if (atLeastOneBaseInterface && coinToss())
					  return "I" + to_string(m_rand() % m_numInterfaces);
				  else
					  return string{};
			  }
			  else if (interfaceScope() && atLeastOneBaseInterface)
				  return "I" + to_string(m_rand() % m_numInterfaces);
			  else
				  return string{};
			},
			randOneToN(3),
			", ",
			true
		);

	string name{};
	if (contractScope())
		name = "C" + to_string(m_numContracts++);
	else if (interfaceScope())
		name = "I" + to_string(m_numInterfaces++);
	else
		name = "L" + to_string(m_numLibraries++);

	Whiskers contractDef(R"(<?abs>abstract </abs><?c>contract</c><?i>interface</i><?l>library</l> <id><?inh> is <iid></inh> {)");

	m_out << contractDef
		("abs", absContractScope())
		("c", contractScope())
		("i", interfaceScope())
		("l", libraryScope())
		("id", name)
		("inh", !libraryScope() && atLeastOneBaseContract && !baseNames.empty())
		("iid", baseNames)
		.render();
	constructorDefined = false;
	if (coinToss())
		visitChildren(_ctx);
	else
	{
		switch (randOneToN(10))
		{
		case 1:
			if ((contractScope() || absContractScope()) && !constructorDefined)
				m_out << "constructor() external {}";
			break;
		case 2:
			if (!interfaceScope())
				m_out << "function f() public {}";
			else
				m_out << "function f() external;";
			break;
		case 3:
			genModifier();
			break;
		case 4:
			if (contractScope() || absContractScope())
				m_out << "fallback() external {}";
			else if (interfaceScope())
				m_out << "fallback() external;";
			break;
		case 5:
			if (contractScope() || absContractScope())
				m_out << "receive() external payable {}";
			else if (interfaceScope())
				m_out << "receive() external payable;";
			break;
		case 6:
			m_out << "struct s { uint x; }";
			break;
		case 7:
			m_out << "enum e { first }";
			break;
		case 8:
			if (contractScope() || absContractScope())
				m_out << "uint x;";
			else if (libraryScope())
				m_out << "uint constant x = 1337;";
			break;
		case 9:
			m_out << "event e() anonymous;";
			break;
		case 10:
			if (!interfaceScope())
				m_out << "using L for *;";
			break;
		}
	}

	m_out << "}\n";
}

antlrcpp::Any SolidityMutator::visitContractDefinition(SolP::ContractDefinitionContext* _ctx)
{
	unsigned numGlobalFuncs = m_numFunctions;
	m_numFunctions = 0;
	ScopeGuard s( [&]() { m_type = Type::GLOBAL; m_numFunctions = numGlobalFuncs; });
	m_type = _ctx->Abstract() ? Type::ABSCONTRACT : Type::CONTRACT;
	interfaceContractLibraryVisitor(_ctx);
	if (coinToss())
		genContract();
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitInterfaceDefinition(SolP::InterfaceDefinitionContext* _ctx)
{
	unsigned numGlobalFuncs = m_numFunctions;
	m_numFunctions = 0;
	ScopeGuard s( [&]() { m_type = Type::GLOBAL; m_numFunctions = numGlobalFuncs; });
	m_type = Type::INTERFACE;
	interfaceContractLibraryVisitor(_ctx);
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitLibraryDefinition(SolP::LibraryDefinitionContext* _ctx)
{
	unsigned numGlobalFuncs = m_numFunctions;
	m_numFunctions = 0;
	ScopeGuard s( [&]() { m_type = Type::GLOBAL; m_numFunctions = numGlobalFuncs; });
	m_type = Type::LIBRARY;
	interfaceContractLibraryVisitor(_ctx);
	return antlrcpp::Any();
}

template <typename T>
void SolidityMutator::visitItemOrGenerate(T _item, function<void()> _generator)
{
	if (_item)
		_item->accept(this);
	else
		_generator();
}

antlrcpp::Any SolidityMutator::visitContractBodyElement(SolP::ContractBodyElementContext* _ctx)
{
	return genericVisitor(_ctx, "contractbodyelement");
}

antlrcpp::Any SolidityMutator::visitTypeName(SolP::TypeNameContext*)
{
	m_out << typeName();
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitParameterList(SolP::ParameterListContext* _ctx)
{
	vector<string> paramDecls{};
	for (auto &i: _ctx->parameterDeclaration())
		paramDecls.push_back(i->accept(this).as<string>());
	generate_n(
		back_inserter(paramDecls),
		randOneToN(3),
		[&]() {
			return Whiskers(R"(<typeName> <dataLoc> <id>)")
				("typeName", typeName())
				("dataLoc", dataLocation())
				("id", genRandomId("p", 10))
				.render();
		}
    );
	return boost::algorithm::join(paramDecls, ", ");
}

antlrcpp::Any SolidityMutator::visitParameterDeclaration(SolP::ParameterDeclarationContext* _ctx)
{
	Whiskers paramDecl(R"(<typeName> <dataLoc> <id>)");
	return paramDecl("typeName", typeName())("dataLoc", dataLocation())("id", genRandomId("p", 10)).render();
}

antlrcpp::Any SolidityMutator::visitConstructorDefinition(SolP::ConstructorDefinitionContext* _ctx)
{
	// Skip over redundant constructor defs
	if (!constructorDefined)
	{
		Whiskers constructorDef(R"(constructor (<args>) <payable> <visibility>)");
		string args = _ctx->arguments ? _ctx->arguments->accept(this).as<string>() : "";
		string visibility = coinToss() ? "public" : "internal";
		string payable = coinToss() ? "payable" : "";
		m_out << constructorDef("args", args)("payable", payable)("visibility", visibility).render();
		constructorDefined = true;
		if (_ctx->body)
			_ctx->body->accept(this);
		else
			m_out << "{}";
	}
	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitEventParameter(SolP::EventParameterContext* _ctx)
{
	return eventParam();
}

antlrcpp::Any SolidityMutator::visitEventDefinition(SolP::EventDefinitionContext* _ctx)
{
	Whiskers eventDef(R"(event <id>(<params>) <anonymous>;))");
	eventDef("id", genRandomId("ev"));

	vector<string> params{};
	for (auto &i: _ctx->parameters)
		params.push_back(i->accept(this).as<string>());
	eventDef("params", boost::algorithm::join(params, ", "));
	eventDef("anonymous", coinToss() ? "anonymous" : "");

	m_out << eventDef.render();
	vector<string> addEvents;
	generate_n(
		back_inserter(addEvents),
		randOneToN(3),
		[&]() {
			Whiskers t = Whiskers(R"(event <id>(<params>) <anon>;)");
			vector<string> params{};
			generate_n(
				back_inserter(params),
				randOneToN(3),
				[&]() { return eventParam(); }
			);
			return t
				("id", genRandomId("p", 10))
				("params", boost::algorithm::join(params, ", "))
				("anon", coinToss() ? "anonymous" : "")
				.render();
		}
	);
	m_out << boost::algorithm::join(addEvents, "");
	return antlrcpp::Any();
}

string SolidityMutator::genFunctionName()
{
	if (likely())
		return "f" + to_string(m_numFunctions++);
	else
	{
		if (coinToss())
			return "fallback";
		else
			return "receive";
	}
}

void SolidityMutator::genModifier()
{
	Whiskers modDef(R"(modifier <id> (<params>) <virtual> <override> <SemicolonOrBody>)");
	modDef("id", "m" + to_string(m_numModifiers++));
	modDef("params", "");
	modDef("virtual", coinToss() ? "virtual" : "");
	modDef("override", coinToss() ? "override": "");
	modDef("SemicolonOrBody", coinToss() ? ";" : "{ _; }");
	m_out << modDef.render();
}

antlrcpp::Any SolidityMutator::visitFunctionDefinition(SolP::FunctionDefinitionContext* _ctx)
{
	// Free functions cannot
	// - have visibility
	// - are not payable
	// - are not virtual
	// - cannot be overridden
	string vis{};
	if (interfaceScope())
		vis = "external";
	else if (contractScope() || libraryScope())
		vis = visibility();
	string mut = mutability();
	if ((vis == "internal" || vis == "private") && mut == "payable")
		vis = "public";
	string id = genFunctionName();

	// Must implement
	bool mustImplement = contractScope() || libraryScope() || globalScope();
	// May implement
	bool mayImplement = absContractScope() && coinToss();

	Whiskers functionDef(
		R"(function <id>(<args>) <vis> <mut> <?virt>virtual</virt> <?oride>override</oride> <mod>)"
	);
	bool mayVirtualise = (contractScope() || absContractScope()) && (vis == "public" || vis == "external");
	bool mayOverride = contractScope() || absContractScope() || interfaceScope();
	m_out << functionDef
		("id", id)
		("args", _ctx->arguments ? _ctx->arguments->accept(this).as<string>() : "")
		("vis", vis)
		("mut", mut)
		("virt", mayVirtualise && coinToss())
		("oride", mayOverride && coinToss())
		("mod", "")
		.render();
	if (_ctx->body && (mustImplement || mayImplement))
		_ctx->body->accept(this);
	else
	{
		if (mustImplement || mayImplement)
			m_out << "{}";
		else
			m_out << ";";
	}

	return antlrcpp::Any();
}

antlrcpp::Any SolidityMutator::visitBlock(SolP::BlockContext* _ctx)
{
	m_out << "{}";
	return antlrcpp::Any();
}