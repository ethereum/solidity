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

/// Unit tests for libsolidity/analysis/FunctionCallGraph.h

#include <libsolidity/analysis/FunctionCallGraph.h>

#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolutil/CommonData.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <libsolidity/analysis/ContractLevelChecker.h>
#include <libsolidity/analysis/DeclarationTypeChecker.h>
#include <libsolidity/analysis/DocStringAnalyser.h>
#include <libsolidity/analysis/DocStringTagParser.h>
#include <libsolidity/analysis/NameAndTypeResolver.h>
#include <libsolidity/analysis/PostTypeChecker.h>
#include <libsolidity/analysis/Scoper.h>
#include <libsolidity/analysis/SyntaxChecker.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/parsing/Parser.h>

#include <boost/test/unit_test.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/set_algorithm.hpp>
#include <range/v3/view/transform.hpp>

#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace ranges;
using namespace solidity::langutil;
using namespace solidity::frontend;

using ContractMap = map<string, ContractDefinition const*>;
using EdgeMap = map<
	FunctionCallGraphBuilder::Node,
	set<FunctionCallGraphBuilder::Node, FunctionCallGraphBuilder::CompareByID>,
	FunctionCallGraphBuilder::CompareByID
>;
using EdgeNames = set<tuple<string, string>>;

namespace
{

struct ParsingResult
{
	ASTPointer<SourceUnit> ast;
	shared_ptr<GlobalContext> globalContext;
	ContractMap contractMap;
};

ParsingResult parseAndAnalyzeContracts(string const& _path, string _sourceCode)
{
	EVMVersion evmVersion;
	ErrorList errorList;
	ErrorReporter errorReporter(errorList);
	Parser parser(errorReporter, evmVersion, /* _errorRecovery = */ false);

	auto scanner = make_shared<Scanner>(CharStream(std::move(_sourceCode), _path));
	ASTPointer<SourceUnit> ast = parser.parse(scanner);

	if (!ast)
	{
		soltestAssert(!Error::containsOnlyWarnings(errorReporter.errors()), "Parser returned null but did not report error.");
		return {};
	}

	soltestAssert(
		ranges::all_of(ast->nodes(), [](auto const& _node){ return !dynamic_cast<ImportDirective const*>(_node.get()); }),
		"For simplicity this test suite supports only files without imports."
	);

	// Do only just enough analysis to satisfy FunctionCallGraph's requirements.
	Scoper::assignScopes(*ast);

	auto globalContext = make_shared<GlobalContext>();
	NameAndTypeResolver resolver(*globalContext, evmVersion, errorReporter);

	SyntaxChecker syntaxChecker(errorReporter, /* _useYulOptimizer = */ false);
	DocStringTagParser docStringTagParser(errorReporter);
	DeclarationTypeChecker declarationTypeChecker(errorReporter, evmVersion);
	ContractLevelChecker contractLevelChecker(errorReporter);
	DocStringAnalyser docStringAnalyser(errorReporter);
	TypeChecker typeChecker(evmVersion, errorReporter);
	PostTypeChecker postTypeChecker(errorReporter);

	soltestAssert(syntaxChecker.checkSyntax(*ast), "Syntax check failed.");
	soltestAssert(docStringTagParser.parseDocStrings(*ast), "Docstring tag parser failed.");
	soltestAssert(resolver.registerDeclarations(*ast), "Declaration registration failed.");
	soltestAssert(resolver.performImports(*ast, {}), "Import resolution failed.");
	resolver.warnHomonymDeclarations();
	soltestAssert(resolver.resolveNamesAndTypes(*ast), "Type and name resolution failed.");
	soltestAssert(declarationTypeChecker.check(*ast), "Declaration type check failed.");
	soltestAssert(contractLevelChecker.check(*ast), "Contract-level checks failed.");
	soltestAssert(docStringAnalyser.analyseDocStrings(*ast), "Docstring analysis failed.");
	soltestAssert(typeChecker.checkTypeRequirements(*ast), "Type check failed.");
	soltestAssert(postTypeChecker.check(*ast), "Post type check failed.");
	soltestAssert(postTypeChecker.finalize(), "Post type check failed to finalize.");

	ContractMap contractMap;
	for (ASTPointer<ASTNode> const& node: ast->nodes())
	{
		if (auto const* contract = dynamic_cast<ContractDefinition const*>(node.get()))
		{
			soltestAssert(contractMap.count(contract->name()) == 0, "Contract names in the source are not unique.");
			contractMap[contract->name()] = contract;
		}
	}

	// NOTE: The code in test cases is expected to be correct so we can keep error handling simple
	// here and just assert that there are no compilation errors.
	solAssert(Error::containsOnlyWarnings(errorReporter.errors()), "");

	return {ast, globalContext, std::move(contractMap)};
}

EdgeNames edgeNames(EdgeMap const& _edgeMap)
{
	EdgeNames names;

	for (auto const& [edgeStart, allEnds]: _edgeMap)
		for (auto const& edgeEnd: allEnds)
			names.emplace(toString(edgeStart), toString(edgeEnd));

	return names;
}

void buildGraphsAndCheckExpectations(
	ContractMap const& _parsedContracts,
	map<string, EdgeNames> const& _expectedEdges,
	map<string, set<string>> const& _expectedCreatedContractSets
)
{
	using GraphPtr = unique_ptr<FunctionCallGraphBuilder::ContractCallGraph>;

	auto getName = [](auto const* _contract){ return _contract->name(); };
	auto notEmpty = [](set<string> const& _set){ return !_set.empty(); };

	soltestAssert(
		(_expectedCreatedContractSets | views::values | views::remove_if(notEmpty)).empty(),
		"Contracts that are not expected to create other contracts should not be included in _expectedCreatedContractSets."
	);
	soltestAssert(
		(_parsedContracts | views::keys | to<set>()) == (_expectedEdges | views::keys | to<set>()) &&
		(ranges::views::set_difference(_expectedCreatedContractSets | views::keys, _parsedContracts | views::keys)).empty(),
		"Contracts listed in expectations do not match contracts actually found in the source file."
	);
	for (string const& contractName: _expectedEdges | views::keys)
		soltestAssert(
			(ranges::views::set_difference(valueOrDefault(_expectedCreatedContractSets, contractName, {}), _parsedContracts | views::keys)).empty(),
			"Inconsistent expectations: contract expected to be created but not to be present in the source file."
		);

	map<string, EdgeNames> edges;
	map<string, set<string>> createdContractSets;
	for (string const& contractName: _expectedEdges | views::keys)
	{
		GraphPtr callGraph = FunctionCallGraphBuilder::create(*_parsedContracts.at(contractName));
		edges[contractName] = edgeNames(callGraph->edges);
		if (!callGraph->createdContracts.empty())
			createdContractSets[contractName] = callGraph->createdContracts | views::transform(getName) | to<set<string>>();

		BOOST_TEST(&callGraph->contract == _parsedContracts.at(contractName));
	}

	BOOST_CHECK_EQUAL(edges, _expectedEdges);
	BOOST_CHECK_EQUAL(createdContractSets, _expectedCreatedContractSets);
}

} // namespace

namespace std
{

// TMP: Try to move these operators from std to boost::test_tools::tt_detail where they belong
ostream& operator<<(ostream& _out, EdgeNames const& _edgeNames);
ostream& operator<<(ostream& _out, EdgeNames const& _edgeNames)
{
	for (auto const& edge: _edgeNames | to<vector>() | actions::sort(std::less()))
		_out << "    " << get<0>(edge) << " -> " << get<1>(edge) << endl;
	return _out;
}

ostream& operator<<(ostream& _out, set<string> const& _set);
ostream& operator<<(ostream& _out, set<string> const& _set)
{
	_out << "{" << (_set | views::join(", ") | to<string>()) << "}";
	return _out;
}

ostream& operator<<(ostream& _out, map<string, EdgeNames> const& _edgeSets);
ostream& operator<<(ostream& _out, map<string, EdgeNames> const& _edgeSets)
{
	// Extra newline for error report readability. Otherwise the first line does not start at the first column.
	_out << endl;

	for (auto const &[contractName, edges]: _edgeSets)
	{
		_out << contractName << ":" << endl;
		_out << edges;
	}
	return _out;
}

ostream& operator<<(ostream& _out, map<string, set<string>> const& _map);
ostream& operator<<(ostream& _out, map<string, set<string>> const& _map)
{
	// Extra newline for error report readability. Otherwise the first line does not start at the first column.
	_out << endl;

	for (auto const &[key, value]: _map)
		_out << key << ": " << value << endl;
	return _out;
}

} // namespace std

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(FunctionCallGraphTest)

BOOST_AUTO_TEST_CASE(only_definitions)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() {}

		library L {
			function ext() external {}
			function pub() public {}
			function inr() internal {}
			function prv() private {}
		}

		contract C {
			function ext() external {}
			function pub() public {}
			function inr() internal {}
			function prv() private {}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"Entry", "function C.pub()"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(ordinary_calls)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() {}

		library L {
			function ext() external { pub(); inr(); }
			function pub() public { inr(); }
			function inr() internal { prv(); }
			function prv() private { free(); free(); }
		}

		contract C {
			function ext() external { pub(); }
			function pub() public { inr(); prv(); free(); }
			function inr() internal { prv(); L.inr(); }
			function prv() private { free(); free(); }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"Entry", "function C.pub()"},
			{"function C.ext()", "function C.pub()"},
			{"function C.pub()", "function C.inr()"},
			{"function C.pub()", "function C.prv()"},
			{"function C.pub()", "function free()"},
			{"function C.inr()", "function C.prv()"},
			{"function C.inr()", "function L.inr()"},
			{"function C.prv()", "function free()"},
			{"function L.inr()", "function L.prv()"},
			{"function L.prv()", "function free()"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
			{"function L.ext()", "function L.pub()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.pub()", "function L.inr()"},
			{"function L.inr()", "function L.prv()"},
			{"function L.prv()", "function free()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(call_chains_through_externals)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		library L {
			function ext() external { C(address(0x0)).ext(); }
			function pub() public {}
			function inr() internal {}
			function prv() private {}
		}

		contract C {
			function ext() external {}
			function pub() public {}
			function inr() internal {}
			function prv() private {}

			function ext2() external { this.ext(); this.pub(); L.ext(); L.pub(); }
			function pub2() public { this.ext(); this.pub(); L.ext(); L.pub(); }
			function pub3() public { C(address(0x0)).ext(); }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.pub()"},
			{"Entry", "function C.pub2()"},
			{"Entry", "function C.pub3()"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(calls_from_constructors)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() returns (uint) {}

		library L {
			function ext() external {}
		}

		contract C {
			constructor() { this.ext(); inr(); L.ext(); free(); }

			function ext() external {}
			function inr() internal {}
		}

		contract D {
			uint a = this.ext();
			uint b = inr();
			uint c = free();

			function ext() external returns (uint) {}
			function inr() internal returns (uint) {}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"EntryCreation", "constructor of C"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function D.ext()"},
			{"EntryCreation", "function D.inr()"},
			{"EntryCreation", "function free()"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(calls_to_constructors)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() { new D(); }

		library L {
			function ext() external { new C(); new D(); inr(); }
			function inr() internal { new C(); new D(); free(); }
		}

		contract C {
			constructor() { new D(); }

			function ext() external { new D(); inr(); }
			function inr() internal { new D(); free(); }
		}

		contract D {}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"EntryCreation", "constructor of C"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.inr()", "function free()"},
		}},
	};
	map<string, set<string>> expectedCreatedContracts = {
		{"C", {"D"}},
		{"L", {"C", "D"}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, expectedCreatedContracts);
}

BOOST_AUTO_TEST_CASE(inherited_constructors)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() {}

		library L {
			function ext() external { inr(); }
			function inr() internal { free(); }
		}

		contract C {
			constructor() { inr(); free(); }

			function ext() external { inr(); }
			function inr() internal { free(); }
		}

		contract D {
			constructor() { L.ext(); }
		}

		contract E is C {}

		contract F is C, D(), E {
			constructor() E() C() {}
		}

		contract G is E() {
			function ext2() external { new F(); }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"EntryCreation", "constructor of C"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"EntryCreation", "constructor of D"},
		}},
		{"E", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"EntryCreation", "constructor of C"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"F", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"EntryCreation", "constructor of C"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
			{"constructor of C", "constructor of D"},
			{"constructor of D", "constructor of F"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"G", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext()"},
			{"Entry", "function G.ext2()"},
			{"EntryCreation", "constructor of C"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
			{"EntryCreation", "function C.inr()"},
			{"EntryCreation", "function free()"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.inr()", "function free()"},
		}},
	};
	map<string, set<string>> expectedCreatedContracts = {
		{"G", {"F"}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, expectedCreatedContracts);
}

BOOST_AUTO_TEST_CASE(inherited_functions_virtual_and_super)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		contract C {
			function f() internal {}
			function g() internal virtual {}
			function h() internal virtual {}
		}

		contract D {
			function h() internal virtual {}
		}

		contract E is C, D {
			function g() internal override {}
			function h() internal override(C, D) {}
			function i() internal {}

			function callF() external { f(); }
			function callG() external { g(); }
			function callH() external { h(); }
			function callI() external { i(); }
			function callCF() external { C.f(); }
			function callCG() external { C.g(); }
			function callCH() external { C.h(); }
			function callDH() external { D.h(); }
			function callEI() external { E.i(); }
			function callSuperF() external { super.f(); }
			function callSuperG() external { super.g(); }
			function callSuperH() external { super.h(); }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"D", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"E", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function E.callF()"},
			{"Entry", "function E.callG()"},
			{"Entry", "function E.callH()"},
			{"Entry", "function E.callI()"},
			{"Entry", "function E.callCF()"},
			{"Entry", "function E.callCG()"},
			{"Entry", "function E.callCH()"},
			{"Entry", "function E.callDH()"},
			{"Entry", "function E.callEI()"},
			{"Entry", "function E.callSuperF()"},
			{"Entry", "function E.callSuperG()"},
			{"Entry", "function E.callSuperH()"},
			{"function E.callF()", "function C.f()"},
			{"function E.callG()", "function E.g()"},
			{"function E.callH()", "function E.h()"},
			{"function E.callI()", "function E.i()"},
			{"function E.callCF()", "function C.f()"},
			{"function E.callCG()", "function C.g()"},
			{"function E.callCH()", "function C.h()"},
			{"function E.callDH()", "function D.h()"},
			{"function E.callEI()", "function E.i()"},
			{"function E.callSuperF()", "function C.f()"},
			{"function E.callSuperG()", "function C.g()"},
			{"function E.callSuperH()", "function D.h()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(overloaded_functions)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		enum E {E1, E2, E3}

		function free() {}
		function free(uint) {}
		function free(bytes memory) {}
		function free(E) {}

		contract C {
			function f(E) internal {}
			function f(bool) external {}
		}

		contract D is C {
			function ext1() external { free(); free(123); free("123"); }
			function ext2() external { f(); f(123); f("123"); }
			function ext3() external { free(E.E2); f(E.E2); }
			function ext4() external { this.f(false); }

			function f() internal {}
			function f(uint) internal {}
			function f(bytes memory) internal {}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.f(bool)"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.f(bool)"},
			{"Entry", "function D.ext1()"},
			{"Entry", "function D.ext2()"},
			{"Entry", "function D.ext3()"},
			{"Entry", "function D.ext4()"},
			{"function D.ext1()", "function free()"},
			{"function D.ext1()", "function free(uint256)"},
			{"function D.ext1()", "function free(bytes)"},
			{"function D.ext2()", "function D.f()"},
			{"function D.ext2()", "function D.f(uint256)"},
			{"function D.ext2()", "function D.f(bytes)"},
			{"function D.ext3()", "function free(enum E)"},
			{"function D.ext3()", "function C.f(enum E)"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(modifiers)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		library L {
			modifier m() { g(); _; }

			function f() m internal {}
			function g() internal {}
		}

		contract C {
			modifier m1() virtual { _; }

			function q() m1 internal virtual { L.f(); }
		}

		contract D is C {
			modifier m2() { q(); _; new C(); }

			function p() m2 internal { C.q(); }
			function q() m2 internal override virtual {}
		}

		contract E is D {
			modifier m1() override { _; }
			modifier m3() { p(); _; }

			constructor() D() m1 E.m3 {}
			function ext() external m1 E.m3 { inr(); }
			function inr() internal m1 E.m3 { L.f(); }

			function q() internal override {}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"D", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"L", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"E", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function E.ext()"},
			{"EntryCreation", "constructor of E"},
			{"EntryCreation", "modifier E.m1"},
			{"EntryCreation", "modifier E.m3"},
			{"function C.q()", "modifier E.m1"},
			{"function C.q()", "function L.f()"},
			{"function D.p()", "modifier D.m2"},
			{"function D.p()", "function C.q()"},
			{"function L.f()", "modifier L.m"},
			{"function E.ext()", "function E.inr()"},
			{"function E.ext()", "modifier E.m1"},
			{"function E.ext()", "modifier E.m3"},
			{"function E.inr()", "modifier E.m1"},
			{"function E.inr()", "modifier E.m3"},
			{"function E.inr()", "function L.f()"},
			{"modifier L.m", "function L.g()"},
			{"modifier D.m2", "function E.q()"},
			{"modifier E.m3", "function D.p()"},
		}},
	};
	map<string, set<string>> expectedCreatedContracts = {
		{"E", {"C"}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, expectedCreatedContracts);
}

BOOST_AUTO_TEST_CASE(events)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free() { emit L.Ev(); }

		library L {
			event Ev();
			event Ev(bytes4, string indexed);

			function ext() external { emit Ev(); }
			function inr() internal { emit Ev(0x12345678, "a"); emit L.Ev(); }
		}

		contract C {
			event EvC(uint) anonymous;

			modifier m() { emit EvC(1); _; }
		}

		contract D is C {
			event EvD1(uint);
			event EvD2(uint);

			function ext() m external { emit D.EvD1(1); emit EvC(1); inr(); }
			function inr() m internal { emit EvD1(1); emit C.EvC(1); L.inr(); free(); EvD2; }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function D.ext()"},
			{"function D.ext()", "event D.EvD1(uint256)"},
			{"function D.ext()", "event C.EvC(uint256)"},
			{"function D.ext()", "function D.inr()"},
			{"function D.ext()", "modifier C.m"},
			{"function D.inr()", "event D.EvD1(uint256)"},
			{"function D.inr()", "event C.EvC(uint256)"},
			{"function D.inr()", "function L.inr()"},
			{"function D.inr()", "function free()"},
			{"function D.inr()", "modifier C.m"},
			{"function L.inr()", "event L.Ev(bytes4,string)"},
			{"function L.inr()", "event L.Ev()"},
			{"function free()",  "event L.Ev()"},
			{"modifier C.m", "event C.EvC(uint256)"},
		}},
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext()"},
			{"function L.ext()", "event L.Ev()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(cycles)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free1() { free1(); }
		function free2() { free3(); }
		function free3() { free2(); }

		library L {
			function inr1() internal { inr1(); }
			function inr2() internal { inr3(); }
			function inr3() internal { inr2(); }
		}

		contract C {
			function virt() internal virtual { virt(); }
		}

		contract D is C {
			function init() external { this.ext1(); inr1(); inr2(); L.inr1(); L.inr2(); free1(); free2(); virt(); }

			function ext1() external { this.ext1(); }
			function ext2() external { this.ext3(); }
			function ext3() external { this.ext2(); }
			function inr1() internal { inr1(); }
			function inr2() internal { inr3(); }
			function inr3() internal { inr2(); }
			function inr3(uint) internal {}

			function virt() internal override { C.virt(); }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"L", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"C", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function D.init()"},
			{"Entry", "function D.ext1()"},
			{"Entry", "function D.ext2()"},
			{"Entry", "function D.ext3()"},
			{"function D.init()", "function D.inr1()"},
			{"function D.init()", "function D.inr2()"},
			{"function D.init()", "function D.virt()"},
			{"function D.init()", "function L.inr1()"},
			{"function D.init()", "function L.inr2()"},
			{"function D.init()", "function free1()"},
			{"function D.init()", "function free2()"},
			{"function D.inr1()", "function D.inr1()"},
			{"function D.inr2()", "function D.inr3()"},
			{"function D.inr3()", "function D.inr2()"},
			{"function D.virt()", "function C.virt()"},
			{"function C.virt()", "function D.virt()"},
			{"function L.inr1()", "function L.inr1()"},
			{"function L.inr2()", "function L.inr3()"},
			{"function L.inr3()", "function L.inr2()"},
			{"function free1()", "function free1()"},
			{"function free2()", "function free3()"},
			{"function free3()", "function free2()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(interfaces_and_abstract_contracts)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		interface I {
			event Ev(uint);
			modifier m() virtual;

			function ext1() external;
			function ext2() external;
		}

		interface J is I {
			function ext2() external override;
			function ext3() external;
		}

		abstract contract C is J {
			function ext3() external override virtual;
			function ext4() external { inr2();}
			function inr1() internal virtual;
			function inr2() m internal { inr1(); this.ext1(); this.ext2(); this.ext3(); }
		}

		contract D is C {
			function ext1() public override { emit I.Ev(1); inr1(); inr2(); }
			function ext2() external override { I(this).ext1(); }
			function ext3() external override {}
			function inr1() internal override {}

			modifier m() override { _; }
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"I", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function I.ext1()"},
			{"Entry", "function I.ext2()"},
		}},
		{"J", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function I.ext1()"},
			{"Entry", "function J.ext2()"},
			{"Entry", "function J.ext3()"},
		}},
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function I.ext1()"},
			{"Entry", "function J.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.ext4()"},
			{"function C.ext4()", "function C.inr2()"},
			{"function C.inr2()", "function C.inr1()"},
			{"function C.inr2()", "modifier I.m"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function D.ext1()"},
			{"Entry", "function D.ext2()"},
			{"Entry", "function D.ext3()"},
			{"Entry", "function C.ext4()"},
			{"function C.ext4()", "function C.inr2()"},
			{"function C.inr2()", "function D.inr1()"},
			{"function C.inr2()", "modifier D.m"},
			{"function D.ext1()", "event I.Ev(uint256)"},
			{"function D.ext1()", "function D.inr1()"},
			{"function D.ext1()", "function C.inr2()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(indirect_calls)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free1() {}
		function free2() {}
		function free3() {}

		library L {
			function ext() external {}
			function inr1() internal {}
			function inr2() internal {}
			function inr3() internal {}

			function access() public {
				free1;
				inr1;
				L.ext;
			}

			function expression() public {
				(free2)();
				(inr2)();
			}
		}

		contract C {
			function ext1() external {}
			function ext2() external {}
			function ext3() external {}
			function inr1() internal {}
			function inr2() internal {}
			function inr3() internal {}

			function access() public {
				this.ext1;
				inr1;
				free1;
				L.inr1;
				L.ext;
			}

			function expression() public {
				(this.ext2)();
				(inr2)();
				(free2)();
				(L.inr2)();
				(L.ext)();
			}
		}

		contract D is C {
			constructor() {
				access();
				expression();
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function L.inr1()"},
			{"InternalDispatch", "function L.inr2()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function free2()"},
			{"Entry", "function L.ext()"},
			{"Entry", "function L.access()"},
			{"Entry", "function L.expression()"},
			{"function L.expression()", "InternalDispatch"},
		}},
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function C.inr1()"},
			{"InternalDispatch", "function C.inr2()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function free2()"},
			{"InternalDispatch", "function L.inr1()"},
			{"InternalDispatch", "function L.inr2()"},
			{"Entry", "function C.ext1()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.access()"},
			{"Entry", "function C.expression()"},
			{"function C.expression()", "InternalDispatch"},
		}},
		{"D", {
			{"InternalCreationDispatch", "function L.inr2()"},
			{"InternalCreationDispatch", "function C.inr1()"},
			{"InternalCreationDispatch", "function C.inr2()"},
			{"InternalCreationDispatch", "function free1()"},
			{"InternalCreationDispatch", "function free2()"},
			{"InternalCreationDispatch", "function L.inr1()"},
			{"InternalCreationDispatch", "function L.inr2()"},
			{"InternalDispatch", "InternalCreationDispatch"},
			// NOTE: These three edges from InternalDispatch are only here because the graph builder
			// visits C.access() twice (once from constructor and then again as a part of the contract
			// interface) and each time the current dispatch is different. Functions that do not
			// perform any calls are currently never detected as already visited.
			{"InternalDispatch", "function C.inr1()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function L.inr1()"},
			{"Entry", "function C.ext1()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.access()"},
			{"Entry", "function C.expression()"},
			{"EntryCreation", "constructor of D"},
			{"EntryCreation", "function C.access()"},
			{"EntryCreation", "function C.expression()"},
			{"function C.expression()", "InternalCreationDispatch"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(calls_via_pointers)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		function free1() {}
		function free2() {}
		function free3() {}

		library L {
			function inr1() internal {}
			function inr2() internal {}
			function inr3() internal {}

			function callPtrs(
				function () external e,
				function () internal i,
				function () internal f,
				function () internal l
			) internal
			{
				e();
				i();
				f();
				l();
			}
		}

		contract C {
			function ext1() external {}
			function ext2() external {}
			function ext3() external {}
			function inr1() internal {}
			function inr2() internal {}
			function inr3() internal {}

			function getPtrs2() internal returns (
				function () external,
				function () internal,
				function () internal,
				function () internal
			)
			{
				return (this.ext2, inr2, free2, L.inr2);
			}

			function testLocalVars() public {
				(function () external e, function () i, function () f, function () l) = getPtrs2();
				L.callPtrs(e, i, f, l);
			}
		}

		contract D is C {
			function () external m_e = this.ext1;
			function () internal m_i = inr1;
			function () internal m_f = free1;
			function () internal m_l = L.inr1;
			function () internal immutable m_imm = inr1;

			function callStatePtrs() internal {
				m_e();
				m_i();
				m_f();
				m_l();
			}

			function updateStatePtrs(
				function () external e,
				function () internal i,
				function () internal f,
				function () internal l
			) internal
			{
				m_e = e;
				m_i = i;
				m_f = f;
				m_l = l;
			}

			function testStateVars() public {
				(function () external e, function () i, function () f, function () l) = getPtrs2();
				updateStatePtrs(e, i, f, l);
				callStatePtrs();
			}

			function testImmutablePtr() public {
				m_imm();
			}

			constructor() {
				testStateVars();
				testLocalVars();
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"L", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function C.inr2()"},
			{"InternalDispatch", "function L.inr2()"},
			{"InternalDispatch", "function free2()"},
			{"Entry", "function C.ext1()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.testLocalVars()"},
			{"function C.testLocalVars()", "function C.getPtrs2()"},
			{"function C.testLocalVars()", "function L.callPtrs(function () external,function (),function (),function ())"},
			{"function L.callPtrs(function () external,function (),function (),function ())", "InternalDispatch"},
		}},
		{"D", {
			{"InternalCreationDispatch", "function C.inr1()"},
			{"InternalCreationDispatch", "function C.inr2()"},
			{"InternalCreationDispatch", "function L.inr1()"},
			{"InternalCreationDispatch", "function L.inr2()"},
			{"InternalCreationDispatch", "function free1()"},
			{"InternalCreationDispatch", "function free2()"},
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.ext1()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.testLocalVars()"},
			{"Entry", "function D.testStateVars()"},
			{"Entry", "function D.testImmutablePtr()"},
			{"EntryCreation", "constructor of D"},
			{"EntryCreation", "function C.testLocalVars()"},
			{"EntryCreation", "function D.testStateVars()"},
			{"function C.testLocalVars()", "function C.getPtrs2()"},
			{"function C.testLocalVars()", "function L.callPtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function C.getPtrs2()"},
			{"function D.testStateVars()", "function D.updateStatePtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function D.callStatePtrs()"},
			{"function D.testImmutablePtr()", "InternalDispatch"},
			{"function D.callStatePtrs()", "InternalCreationDispatch"},
			{"function L.callPtrs(function () external,function (),function (),function ())", "InternalCreationDispatch"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(pointer_to_overridden_function)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		contract C {
			function f() internal virtual {}
		}

		contract D is C {
			function f() internal override {}

			function getF() internal returns (function ()) {
				return C.f;
			}

			function getSuperF() internal returns (function ()) {
				return super.f;
			}

			function test1() public {
				getF()();
			}

			function test2() public {
				getSuperF()();
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {{"InternalDispatch", "InternalCreationDispatch"}}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function C.f()"},
			{"Entry", "function D.test1()"},
			{"Entry", "function D.test2()"},
			{"function D.test1()", "function D.getF()"},
			{"function D.test1()", "InternalDispatch"},
			{"function D.test2()", "function D.getSuperF()"},
			{"function D.test2()", "InternalDispatch"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(pointer_to_nonexistent_function)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		interface I {
			function f() external;
		}

		abstract contract C is I {
			function g() internal virtual;

			function getF() internal returns (function () external) { return this.f; }
			function getG() internal returns (function () internal) { return g; }

			function testInterface() public {
				getF()();
				getG()();
			}

			function testBadPtr() public {
				function () ptr;
				ptr();
			}
		}

		contract D is C {
			function f() public override {}
			function g() internal override {}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"I", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function I.f()"},
			  }},
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function C.g()"},
			{"Entry", "function C.testInterface()"},
			{"Entry", "function C.testBadPtr()"},
			{"Entry", "function I.f()"},
			{"function C.testInterface()", "function C.getF()"},
			{"function C.testInterface()", "function C.getG()"},
			{"function C.testInterface()", "InternalDispatch"},
			{"function C.testBadPtr()", "InternalDispatch"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"InternalDispatch", "function D.g()"},
			{"Entry", "function C.testInterface()"},
			{"Entry", "function C.testBadPtr()"},
			{"Entry", "function D.f()"},
			{"function C.testInterface()", "function C.getF()"},
			{"function C.testInterface()", "function C.getG()"},
			{"function C.testInterface()", "InternalDispatch"},
			{"function C.testBadPtr()", "InternalDispatch"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(pointer_cycle)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		contract C {
			function () ptr = f;

			function f() internal { ptr(); }

			function test() public {
				ptr();
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalCreationDispatch", "function C.f()"},
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.test()"},
			{"function C.test()", "InternalDispatch"},
			{"function C.f()", "InternalCreationDispatch"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(using_for)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		struct S {
			uint x;
		}

		library L {
			function ext(S memory _s) external {}
			function inr(S memory _s) internal {}
		}

		contract C {
			using L for S;

			function test() public {
				S memory s = S(42);

				s.ext();
				s.inr();
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"L", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function L.ext(struct S)"},
		}},
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.test()"},
			{"function C.test()", "function L.inr(struct S)"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(getters)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		contract C {
			uint public variable;
			uint[][] public array;
			mapping(bytes => bytes) public map;

			function test() public {
				this.variable();
				this.array(1, 2);
				this.map("value");
			}
		}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "function C.test()"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_CASE(fallback_and_receive)
{
	ParsingResult parsingResult = parseAndAnalyzeContracts(boost::unit_test::framework::current_test_case().p_name, R"(
		contract C {
			fallback() external {}
			receive() external payable {}
		}

		contract D {
			fallback(bytes calldata) external returns (bytes memory){}

			function test() public {
				(bool success, bytes memory result) = address(this).call("abc");
			}
		}

		contract E is C {}
	)"s);

	map<string, EdgeNames> expectedEdges = {
		{"C", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "receive of C"},
			{"Entry", "fallback of C"},
		}},
		{"D", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "fallback of D"},
			{"Entry", "function D.test()"},
		}},
		{"E", {
			{"InternalDispatch", "InternalCreationDispatch"},
			{"Entry", "receive of C"},
			{"Entry", "fallback of C"},
		}},
	};

	buildGraphsAndCheckExpectations(parsingResult.contractMap, expectedEdges, {});
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
