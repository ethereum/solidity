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

#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/CompilerStack.h>

#include <boost/test/unit_test.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/set_algorithm.hpp>
#include <range/v3/view/transform.hpp>

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace std::string_literals;

using EdgeMap = std::map<
	CallGraph::Node,
	std::set<CallGraph::Node, CallGraph::CompareByID>,
	CallGraph::CompareByID
>;
using EdgeNames = std::set<std::tuple<std::string, std::string>>;
using CallGraphMap = std::map<std::string, CallGraph const*>;

namespace
{

std::unique_ptr<CompilerStack> parseAndAnalyzeContracts(std::string _sourceCode)
{
	ReadCallback::Callback fileReader = [](std::string const&, std::string const&)
	{
		soltestAssert(false, "For simplicity this test suite supports only files without imports.");
		return ReadCallback::Result{true, ""};
	};

	auto compilerStack = std::make_unique<CompilerStack>(fileReader);
	compilerStack->setSources({{"", _sourceCode}});

	// NOTE: The code in test cases is expected to be correct so we can keep error handling simple
	// here and just assert that there are no errors.
	bool success = compilerStack->parseAndAnalyze();
	soltestAssert(success, "");

	soltestAssert(
		ranges::all_of(
			compilerStack->ast("").nodes(),
			[](auto const& _node){ return !dynamic_cast<ImportDirective const*>(_node.get()); }
		),
		"For simplicity this test suite supports only files without imports."
	);

	return compilerStack;
}

EdgeNames edgeNames(EdgeMap const& _edgeMap)
{
	EdgeNames names;

	for (auto const& [edgeStart, allEnds]: _edgeMap)
		for (auto const& edgeEnd: allEnds)
			names.emplace(toString(edgeStart), toString(edgeEnd));

	return names;
}

std::tuple<CallGraphMap, CallGraphMap> collectGraphs(CompilerStack const& _compilerStack)
{
	soltestAssert(_compilerStack.state() >= CompilerStack::State::AnalysisSuccessful);

	std::tuple<CallGraphMap, CallGraphMap> graphs;

	for (std::string const& fullyQualifiedContractName: _compilerStack.contractNames())
	{
		soltestAssert(std::get<0>(graphs).count(fullyQualifiedContractName) == 0 && std::get<1>(graphs).count(fullyQualifiedContractName) == 0, "");

		// This relies on two assumptions: (1) CompilerStack received an empty string as a path for
		// the contract and (2) contracts used in test cases have no imports.
		soltestAssert(fullyQualifiedContractName.size() > 0 && fullyQualifiedContractName[0] == ':', "");
		std::string contractName = fullyQualifiedContractName.substr(1);

		std::get<0>(graphs).emplace(contractName, _compilerStack.contractDefinition(fullyQualifiedContractName).annotation().creationCallGraph->get());
		std::get<1>(graphs).emplace(contractName, _compilerStack.contractDefinition(fullyQualifiedContractName).annotation().deployedCallGraph->get());
	}

	return graphs;
}

void checkCallGraphExpectations(
	CallGraphMap const& _callGraphs,
	std::map<std::string, EdgeNames> const& _expectedEdges,
	std::map<std::string, std::set<std::string>> const& _expectedCreatedContractSets = {},
	std::map<std::string, std::set<std::string>> const& _expectedEmittedEventSets = {}
)
{
	auto getContractName = [](ContractDefinition const* _contract){ return _contract->name(); };
	auto eventToString = [](EventDefinition const* _event){ return toString(CallGraph::Node(_event)); };
	auto notEmpty = [](std::set<std::string> const& _set){ return !_set.empty(); };

	soltestAssert(
		(_expectedCreatedContractSets | ranges::views::values | ranges::views::remove_if(notEmpty)).empty(),
		"Contracts that are not expected to create other contracts should not be included in _expectedCreatedContractSets."
	);
	soltestAssert(
		(_expectedEdges | ranges::views::keys | ranges::to<std::set>()) == (_callGraphs | ranges::views::keys | ranges::to<std::set>()) &&
		(ranges::views::set_difference(_expectedCreatedContractSets | ranges::views::keys, _expectedEdges | ranges::views::keys)).empty(),
		"Contracts listed in expectations do not match contracts actually found in the source file or in other expectations."
	);
	for (std::string const& contractName: _expectedEdges | ranges::views::keys)
	{
		soltestAssert(
			(ranges::views::set_difference(valueOrDefault(_expectedCreatedContractSets, contractName, {}), _expectedEdges | ranges::views::keys)).empty(),
			"Inconsistent expectations: contract expected to be created but not to be present in the source file."
		);
	}

	std::map<std::string, EdgeNames> edges;
	std::map<std::string, std::set<std::string>> createdContractSets;
	std::map<std::string, std::set<std::string>> emittedEventSets;
	for (std::string const& contractName: _expectedEdges | ranges::views::keys)
	{
		soltestAssert(_callGraphs.at(contractName) != nullptr, "");
		CallGraph const& callGraph = *_callGraphs.at(contractName);

		edges[contractName] = edgeNames(callGraph.edges);
		if (!callGraph.bytecodeDependency.empty())
			createdContractSets[contractName] = callGraph.bytecodeDependency |
				ranges::views::keys |
				ranges::views::transform(getContractName) |
				ranges::to<std::set<std::string>>();
		if (!callGraph.emittedEvents.empty())
			emittedEventSets[contractName] = callGraph.emittedEvents |
				 ranges::views::transform(eventToString) |
				 ranges::to<std::set<std::string>>();
	}

	BOOST_CHECK_EQUAL(edges, _expectedEdges);
	BOOST_CHECK_EQUAL(createdContractSets, _expectedCreatedContractSets);
	BOOST_CHECK_EQUAL(emittedEventSets, _expectedEmittedEventSets);
}

std::ostream& operator<<(std::ostream& _out, EdgeNames const& _edgeNames)
{
	for (auto const& [from, to]: _edgeNames)
		_out << "    " << from << " -> " << to << std::endl;
	return _out;
}

std::ostream& operator<<(std::ostream& _out, std::set<std::string> const& _set)
{
	_out << "{" << (_set | ranges::views::join(", ") | ranges::to<std::string>()) << "}";
	return _out;
}

std::ostream& operator<<(std::ostream& _out, std::map<std::string, EdgeNames> const& _edgeSets)
{
	// Extra newline for error report readability. Otherwise the first line does not start at the first column.
	_out << std::endl;

	for (auto const &[contractName, edges]: _edgeSets)
	{
		_out << contractName << ":" << std::endl;
		_out << edges;
	}
	return _out;
}

std::ostream& operator<<(std::ostream& _out, std::map<std::string, std::set<std::string>> const& _map)
{
	// Extra newline for error report readability. Otherwise the first line does not start at the first column.
	_out << std::endl;

	for (auto const &[key, value]: _map)
		_out << key << ": " << value << std::endl;
	return _out;
}

} // namespace

namespace boost::test_tools::tt_detail
{

// Boost won't find the << operator unless we put it in the std namespace which is illegal.
// The recommended solution is to overload print_log_value<> struct and make it use our operator.

template<>
struct print_log_value<EdgeNames>
{
	void operator()(std::ostream& _output, EdgeNames const& _edgeNames) { ::operator<<(_output, _edgeNames); }
};

template<>
struct print_log_value<std::set<std::string>>
{
	void operator()(std::ostream& _output, std::set<std::string> const& _set) { ::operator<<(_output, _set); }
};

template<>
struct print_log_value<std::map<std::string, EdgeNames>>
{
	void operator()(std::ostream& _output, std::map<std::string, EdgeNames> const& _edgeSets) { ::operator<<(_output, _edgeSets); }
};

template<>
struct print_log_value<std::map<std::string, std::set<std::string>>>
{
	void operator()(std::ostream& _output, std::map<std::string, std::set<std::string>> const& _map) { ::operator<<(_output, _map); }
};

} // namespace boost::test_tools::tt_detail

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(FunctionCallGraphTest)

BOOST_AUTO_TEST_CASE(only_definitions)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
			{"Entry", "function C.pub()"},
		}},
		{"L", {
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(ordinary_calls)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
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
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
			{"function L.ext()", "function L.pub()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.pub()", "function L.inr()"},
			{"function L.inr()", "function L.prv()"},
			{"function L.prv()", "function free()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(call_chains_through_externals)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.pub()"},
			{"Entry", "function C.pub2()"},
			{"Entry", "function C.pub3()"},
		}},
		{"L", {
			{"Entry", "function L.ext()"},
			{"Entry", "function L.pub()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(calls_from_constructors)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {
			{"Entry", "constructor of C"},
			{"constructor of C", "function C.inr()"},
			{"constructor of C", "function free()"},
		}},
		{"D", {
			{"Entry", "function D.inr()"},
			{"Entry", "function free()"},
		}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
		}},
		{"D", {
			{"Entry", "function D.ext()"},
		}},
		{"L", {
			{"Entry", "function L.ext()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(calls_to_constructors)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {
			{"Entry", "constructor of C"},
		}},
		{"D", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
			{"function C.ext()", "function C.inr()"},
			{"function C.inr()", "function free()"},
		}},
		{"D", {}},
		{"L", {
			{"Entry", "function L.ext()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.inr()", "function free()"},
		}},
	};

	std::map<std::string, std::set<std::string>> expectedCreatedContractsAtCreation = {
		{"C", {"D"}},
	};
	std::map<std::string, std::set<std::string>> expectedCreatedContractsAfterDeployment = {
		{"C", {"D"}},
		{"L", {"C", "D"}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges, expectedCreatedContractsAtCreation);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges, expectedCreatedContractsAfterDeployment);
}

BOOST_AUTO_TEST_CASE(inherited_constructors)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		function free() {}

		library L {
			function ext() external { inr(); }
			function inr() internal { free(); }
		}

		contract C {
			constructor() { inrC(); free(); }

			function extC() external returns (uint) { inrC(); }
			function inrC() internal returns (uint) { free(); }
		}

		contract D {
			constructor() { L.ext(); }
		}

		contract E is C {
			uint e2 = this.extE();
			uint i2 = inrE();

			function extE() external returns (uint) { inrE(); }
			function inrE() internal returns (uint) { free(); }
		}

		contract F is C, D(), E {
			uint e3 = this.extF();
			uint i3 = inrF();

			constructor() E() C() {}

			function extF() external returns (uint) { inrF(); }
			function inrF() internal returns (uint) { free(); }
		}

		contract G is E() {
			function extG() external returns (uint) { new F(); }
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {
			{"Entry", "constructor of C"},
			{"constructor of C", "function C.inrC()"},
			{"constructor of C", "function free()"},
			{"function C.inrC()", "function free()"},
		}},
		{"D", {
			{"Entry", "constructor of D"},
		}},
		{"E", {
			{"Entry", "constructor of C"},
			{"Entry", "function E.inrE()"},
			{"constructor of C", "function C.inrC()"},
			{"constructor of C", "function free()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
		}},
		{"F", {
			{"Entry", "constructor of C"},
			{"Entry", "constructor of D"},
			{"Entry", "constructor of F"},
			{"Entry", "function E.inrE()"},
			{"Entry", "function F.inrF()"},
			{"constructor of C", "function C.inrC()"},
			{"constructor of C", "function free()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
			{"function F.inrF()", "function free()"},
		}},
		{"G", {
			{"Entry", "constructor of C"},
			{"Entry", "function E.inrE()"},
			{"constructor of C", "function C.inrC()"},
			{"constructor of C", "function free()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
		}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.extC()"},
			{"function C.extC()", "function C.inrC()"},
			{"function C.inrC()", "function free()"},
		}},
		{"D", {}},
		{"E", {
			{"Entry", "function C.extC()"},
			{"Entry", "function E.extE()"},
			{"function C.extC()", "function C.inrC()"},
			{"function E.extE()", "function E.inrE()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
		}},
		{"F", {
			{"Entry", "function C.extC()"},
			{"Entry", "function E.extE()"},
			{"Entry", "function F.extF()"},
			{"function C.extC()", "function C.inrC()"},
			{"function E.extE()", "function E.inrE()"},
			{"function F.extF()", "function F.inrF()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
			{"function F.inrF()", "function free()"},
		}},
		{"G", {
			{"Entry", "function C.extC()"},
			{"Entry", "function E.extE()"},
			{"Entry", "function G.extG()"},
			{"function C.extC()", "function C.inrC()"},
			{"function E.extE()", "function E.inrE()"},
			{"function C.inrC()", "function free()"},
			{"function E.inrE()", "function free()"},
		}},
		{"L", {
			{"Entry", "function L.ext()"},
			{"function L.ext()", "function L.inr()"},
			{"function L.inr()", "function free()"},
		}},
	};

	std::map<std::string, std::set<std::string>> expectedCreatedContractsAtCreation = {};
	std::map<std::string, std::set<std::string>> expectedCreatedContractsAfterDeployment = {
		{"G", {"F"}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges, expectedCreatedContractsAtCreation);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges, expectedCreatedContractsAfterDeployment);
}

BOOST_AUTO_TEST_CASE(inheritance_specifiers)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		function fD() returns (uint) {}
		function fE() returns (uint) {}
		function fFD() returns (uint) {}
		function fFE() returns (uint) {}
		function fG() returns (uint) {}

		function fVarC() returns (uint) {}
		function fVarD() returns (uint) {}
		function fVarE() returns (uint) {}
		function fVarF() returns (uint) {}

		contract C {
			uint c = fVarC();

			constructor (uint) {}
		}

		contract D is C(fD()) {
			uint d = fVarD();

			constructor (uint) {}
		}

		abstract contract E is C {
			uint e = fVarE();

			constructor (uint) {}
		}

		contract F is D(fFD()), E {
			uint f = fVarF();

			constructor (uint) E(fFE()) {}
		}

		contract G is D(fG()), E(fG()) {}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {
			{"Entry", "constructor of C"},
			{"Entry", "function fVarC()"},
		}},
		{"D", {
			{"Entry", "constructor of C"},
			{"Entry", "constructor of D"},
			{"Entry", "function fVarC()"},
			{"Entry", "function fVarD()"},
			{"constructor of D", "function fD()"},
		}},
		{"E", {
			{"Entry", "constructor of C"},
			{"Entry", "constructor of E"},
			{"Entry", "function fVarC()"},
			{"Entry", "function fVarE()"},
		}},
		{"F", {
			{"Entry", "constructor of C"},
			{"Entry", "constructor of D"},
			{"Entry", "constructor of E"},
			{"Entry", "constructor of F"},
			{"Entry", "function fVarC()"},
			{"Entry", "function fVarD()"},
			{"Entry", "function fVarE()"},
			{"Entry", "function fVarF()"},
			{"constructor of D", "function fD()"},
			{"constructor of F", "function fFD()"},
			{"constructor of F", "function fFE()"},
		}},
		{"G", {
			{"Entry", "constructor of C"},
			{"Entry", "constructor of D"},
			{"Entry", "constructor of E"},
			// G, unlike F, has no constructor so fG() gets an edge from Entry.
			{"Entry", "function fG()"},
			{"Entry", "function fVarC()"},
			{"Entry", "function fVarD()"},
			{"Entry", "function fVarE()"},
			{"constructor of D", "function fD()"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {}},
		{"D", {}},
		{"E", {}},
		{"F", {}},
		{"G", {}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(inherited_functions_virtual_and_super)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		contract C {
			function f() internal {}
			function g() internal virtual {}
			function h() internal virtual {}

			function ext() external virtual {}
		}

		contract D {
			function h() internal virtual {}

			function ext() external virtual {}
		}

		contract E is C, D {
			function g() internal override {}
			function h() internal override(C, D) {}
			function i() internal {}

			function ext() external override(C, D) {}

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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
		{"E", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
		}},
		{"D", {
			{"Entry", "function D.ext()"},
		}},
		{"E", {
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
			{"Entry", "function E.ext()"},
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

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(overloaded_functions)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.f(bool)"},
		}},
		{"D", {
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

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(modifiers)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
		{"L", {}},
		{"E", {
			{"Entry", "constructor of E"},
			{"constructor of E", "modifier E.m1"},
			{"constructor of E", "modifier E.m3"},
			{"function C.q()", "modifier E.m1"},
			{"function C.q()", "function L.f()"},
			{"function D.p()", "modifier D.m2"},
			{"function D.p()", "function C.q()"},
			{"function L.f()", "modifier L.m"},
			{"modifier L.m", "function L.g()"},
			{"modifier D.m2", "function E.q()"},
			{"modifier E.m3", "function D.p()"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {}},
		{"D", {}},
		{"L", {}},
		{"E", {
			{"Entry", "function E.ext()"},
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

	std::map<std::string, std::set<std::string>> expectedCreatedContractsAtCreation = {{"E", {"C"}}};
	std::map<std::string, std::set<std::string>> expectedCreatedContractsAfterDeployment = {{"E", {"C"}}};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges, expectedCreatedContractsAtCreation);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges, expectedCreatedContractsAfterDeployment);
}

BOOST_AUTO_TEST_CASE(events)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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

			function ext() m external { emit D.EvD1(1); emit EvC(f()); inr(); }
			function inr() m internal { emit EvD1(1); emit C.EvC(f()); L.inr(); free(); EvD2; }

			function f() internal returns (uint) {}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {}},
		{"D", {
			{"Entry", "function D.ext()"},
			{"function D.ext()", "function D.inr()"},
			{"function D.ext()", "modifier C.m"},
			{"function D.ext()", "function D.f()"},
			{"function D.inr()", "function L.inr()"},
			{"function D.inr()", "function free()"},
			{"function D.inr()", "modifier C.m"},
			{"function D.inr()", "function D.f()"},
		}},
		{"L", {
			{"Entry", "function L.ext()"},
		}},
	};

	std::map<std::string, std::set<std::string>> expectedCreationEvents = {};
	std::map<std::string, std::set<std::string>> expectedDeployedEvents = {
		{"D", {
			"event D.EvD1(uint256)",
			"event C.EvC(uint256)",
			"event L.Ev(bytes4,string)",
			"event L.Ev()",
		}},
		{"L", {
			"event L.Ev()",
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges, {}, expectedCreationEvents);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges, {}, expectedDeployedEvents);
}

BOOST_AUTO_TEST_CASE(cycles)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"L", {}},
		{"C", {}},
		{"D", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"L", {}},
		{"C", {}},
		{"D", {
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

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(interfaces_and_abstract_contracts)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		interface I {
			event Ev(uint);

			function ext1() external;
			function ext2() external;
		}

		interface J is I {
			function ext2() external override;
			function ext3() external;
		}

		abstract contract C is J {
			modifier m() virtual;

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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"I", {}},
		{"J", {}},
		{"C", {}},
		{"D", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"I", {
			{"Entry", "function I.ext1()"},
			{"Entry", "function I.ext2()"},
		}},
		{"J", {
			{"Entry", "function I.ext1()"},
			{"Entry", "function J.ext2()"},
			{"Entry", "function J.ext3()"},
		}},
		{"C", {
			{"Entry", "function I.ext1()"},
			{"Entry", "function J.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.ext4()"},
			{"function C.ext4()", "function C.inr2()"},
			{"function C.inr2()", "function C.inr1()"},
			{"function C.inr2()", "modifier C.m"},
		}},
		{"D", {
			{"Entry", "function D.ext1()"},
			{"Entry", "function D.ext2()"},
			{"Entry", "function D.ext3()"},
			{"Entry", "function C.ext4()"},
			{"function C.ext4()", "function C.inr2()"},
			{"function C.inr2()", "function D.inr1()"},
			{"function C.inr2()", "modifier D.m"},
			{"function D.ext1()", "function D.inr1()"},
			{"function D.ext1()", "function C.inr2()"},
		}},
	};

	std::map<std::string, std::set<std::string>> expectedCreationEvents = {};
	std::map<std::string, std::set<std::string>> expectedDeployedEvents = {
		{"D", {
			"event I.Ev(uint256)",
		}},
	};


	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges, {}, expectedCreationEvents);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges, {}, expectedDeployedEvents);
}

BOOST_AUTO_TEST_CASE(indirect_calls)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"L", {}},
		{"C", {}},
		{"D", {
			{"InternalDispatch", "function L.inr2()"},
			{"InternalDispatch", "function C.inr1()"},
			{"InternalDispatch", "function C.inr2()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function free2()"},
			{"InternalDispatch", "function L.inr1()"},
			{"InternalDispatch", "function L.inr2()"},
			{"Entry", "constructor of D"},
			{"constructor of D", "function C.access()"},
			{"constructor of D", "function C.expression()"},
			{"function C.expression()", "InternalDispatch"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"L", {
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
			{"InternalDispatch", "function L.inr2()"},
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
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(calls_via_pointers)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"L", {}},
		{"C", {}},
		{"D", {
			{"InternalDispatch", "function C.inr1()"},
			{"InternalDispatch", "function C.inr2()"},
			{"InternalDispatch", "function L.inr1()"},
			{"InternalDispatch", "function L.inr2()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function free2()"},
			{"Entry", "constructor of D"},
			{"constructor of D", "function C.testLocalVars()"},
			{"constructor of D", "function D.testStateVars()"},
			{"function C.testLocalVars()", "function C.getPtrs2()"},
			{"function C.testLocalVars()", "function L.callPtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function C.getPtrs2()"},
			{"function D.testStateVars()", "function D.updateStatePtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function D.callStatePtrs()"},
			{"function D.callStatePtrs()", "InternalDispatch"},
			{"function L.callPtrs(function () external,function (),function (),function ())", "InternalDispatch"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"L", {}},
		{"C", {
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
			{"InternalDispatch", "function C.inr1()"},
			{"InternalDispatch", "function C.inr2()"},
			{"InternalDispatch", "function L.inr1()"},
			{"InternalDispatch", "function L.inr2()"},
			{"InternalDispatch", "function free1()"},
			{"InternalDispatch", "function free2()"},
			{"Entry", "function C.ext1()"},
			{"Entry", "function C.ext2()"},
			{"Entry", "function C.ext3()"},
			{"Entry", "function C.testLocalVars()"},
			{"Entry", "function D.testStateVars()"},
			{"Entry", "function D.testImmutablePtr()"},
			{"function C.testLocalVars()", "function C.getPtrs2()"},
			{"function C.testLocalVars()", "function L.callPtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function C.getPtrs2()"},
			{"function D.testStateVars()", "function D.updateStatePtrs(function () external,function (),function (),function ())"},
			{"function D.testStateVars()", "function D.callStatePtrs()"},
			{"function D.testImmutablePtr()", "InternalDispatch"},
			{"function D.callStatePtrs()", "InternalDispatch"},
			{"function L.callPtrs(function () external,function (),function (),function ())", "InternalDispatch"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(pointer_to_overridden_function)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {}},
		{"D", {
			{"InternalDispatch", "function C.f()"},
			{"Entry", "function D.test1()"},
			{"Entry", "function D.test2()"},
			{"function D.test1()", "function D.getF()"},
			{"function D.test1()", "InternalDispatch"},
			{"function D.test2()", "function D.getSuperF()"},
			{"function D.test2()", "InternalDispatch"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(pointer_to_nonexistent_function)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"I", {}},
		{"C", {}},
		{"D", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"I", {
			{"Entry", "function I.f()"},
			  }},
		{"C", {
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

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(function_self_reference)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		contract C {
			function f() public returns (bool ret) {
				return f == f;
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.f()"},
			{"InternalDispatch", "function C.f()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(pointer_cycle)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		contract C {
			function () ptr = f;

			function f() internal { ptr(); }

			function test() public {
				ptr();
			}
		}
	)"s);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {
			{"InternalDispatch", "function C.f()"},
			{"function C.f()", "InternalDispatch"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"InternalDispatch", "function C.f()"},
			{"Entry", "function C.test()"},
			{"function C.test()", "InternalDispatch"},
			{"function C.f()", "InternalDispatch"},
		}},
	};
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(using_for)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		struct S {
			uint x;
		}

		library L {
			function ext(S memory _s) external {}
			function inr(S memory _s) internal {}
		}

		contract C {
			using L for S;

			function pub() public {
				S memory s = S(42);

				s.ext();
				s.inr();
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"L", {}},
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"L", {
			{"Entry", "function L.ext(struct S)"},
		}},
		{"C", {
			{"Entry", "function C.pub()"},
			{"function C.pub()", "function L.inr(struct S)"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(user_defined_binary_operator)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		type Int is int128;
		using {add as +} for Int global;

		function add(Int, Int) pure returns (Int) {
			return Int.wrap(0);
		}

		contract C {
			function pub() public {
				Int.wrap(0) + Int.wrap(1);
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.pub()"},
			{"function C.pub()", "function add(Int,Int)"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(user_defined_unary_operator)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		type Int is int128;
		using {sub as -} for Int global;

		function sub(Int) pure returns (Int) {
			return Int.wrap(0);
		}

		contract C {
			function pub() public {
				-Int.wrap(1);
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.pub()"},
			{"function C.pub()", "function sub(Int)"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(getters)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
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
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {{"Entry", "function C.test()"}}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(fallback_and_receive)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		contract C {
			function ext() external {}
			function inr() internal {}

			fallback() external {
				this.ext();
				inr();
			}

			receive() external payable {
				this.ext();
				inr();
			}
		}

		contract D {
			fallback(bytes calldata) external returns (bytes memory) {}

			function test() public {
				(bool success, bytes memory result) = address(this).call("abc");
			}
		}

		contract E is C {}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
		{"E", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "function C.ext()"},
			{"Entry", "receive of C"},
			{"Entry", "fallback of C"},
			{"fallback of C", "function C.inr()"},
			{"receive of C", "function C.inr()"},
		}},
		{"D", {
			{"Entry", "function D.test()"},
			{"Entry", "fallback of D"},
		}},
		{"E", {
			{"Entry", "function C.ext()"},
			{"Entry", "fallback of C"},
			{"Entry", "receive of C"},
			{"fallback of C", "function C.inr()"},
			{"receive of C", "function C.inr()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(virtual_fallback_and_receive)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		contract C {
			fallback() external virtual {}
			receive() external payable virtual {}
		}

		contract D is C {}

		contract E is D {
			fallback() external virtual override {}
			receive() external payable virtual override {}
		}

		contract F is E {
			fallback() external override {}
			receive() external payable override {}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"C", {}},
		{"D", {}},
		{"E", {}},
		{"F", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"C", {
			{"Entry", "receive of C"},
			{"Entry", "fallback of C"},
		}},
		{"D", {
			{"Entry", "receive of C"},
			{"Entry", "fallback of C"},
		}},
		{"E", {
			{"Entry", "receive of E"},
			{"Entry", "fallback of E"},
		}},
		{"F", {
			{"Entry", "receive of F"},
			{"Entry", "fallback of F"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(builtins)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		interface I {}

		contract C {
			struct S {
				uint x;
			}
			function accessBuiltin() public payable {
				abi.decode;
				abi.encode;
				abi.encodePacked;
				abi.encodeWithSelector;
				abi.encodeWithSignature;
				block.basefee;
				block.chainid;
				block.coinbase;
				block.difficulty;
				block.prevrandao;
				block.gaslimit;
				block.number;
				block.timestamp;
				gasleft;
				msg.data;
				msg.sender;
				msg.value;
				tx.gasprice;
				tx.origin;
				blockhash;
				keccak256;
				sha256;
				ripemd160;
				ecrecover;
				addmod;
				mulmod;
				this;
				super;
				selfdestruct;
				address(0).balance;
				address(0).code;
				address(0).codehash;
				payable(0).send;
				payable(0).transfer;
				address(0).call;
				address(0).delegatecall;
				address(0).staticcall;
				type(C).name;
				type(I).interfaceId;
				type(S).typehash;
				type(uint).min;
				type(uint).max;
				assert;
			}

			function callBuiltin() public payable {
				bytes memory data;

				abi.decode(data, (uint));
				abi.encode(0);
				abi.encodePacked(data);
				abi.encodeWithSelector(0x12345678);
				abi.encodeWithSignature("abc");
				gasleft();
				blockhash(0);
				keccak256(data);
				sha256(data);
				ripemd160(data);
				ecrecover(0x0, 0, 0, 0);
				addmod(1, 2, 3);
				mulmod(1, 2, 3);
				selfdestruct(payable(0));
				payable(0).send(0);
				payable(0).transfer(0);
				address(0).call(data);
				address(0).delegatecall(data);
				address(0).staticcall(data);
				assert(true);
				require(true);
				revert();
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"I", {}},
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"I", {}},
		{"C", {
			{"Entry", "function C.accessBuiltin()"},
			{"Entry", "function C.callBuiltin()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(conversions_and_struct_array_constructors)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		interface I {}

		enum E {A, B, C}

		struct S {
			uint a;
			E b;
		}

		contract C is I {
			uint[] u;

			function convert() public payable {
				uint(0);
				int(0);
				bool(true);
				bytes16(0);
				payable(address(0));
				E(0);
				C(address(C(address(0))));
				I(C(address(0)));

				bytes memory b;
				string(b);
				bytes(b);
			}

			function create() public payable {
				S(1, E.A);

				uint[3] memory u3;
				uint[3](u3);
				uint[](new uint[](3));
			}

			function pushPop() public payable {
				u.push();
				u.push(1);
				u.pop();
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"I", {}},
		{"C", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"I", {}},
		{"C", {
			{"Entry", "function C.convert()"},
			{"Entry", "function C.create()"},
			{"Entry", "function C.pushPop()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(immutable_initialization)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		function free() pure returns (uint) { return 42; }

		contract Base {
			function ext() external pure returns (uint) { free(); }
			function inr() internal pure returns (uint) { free(); }
		}

		contract C is Base {
			uint immutable extImmutable = this.ext();
			uint immutable inrImmutable = inr();
		}

		contract D is Base {
			uint immutable extImmutable;
			uint immutable inrImmutable;

			constructor () {
				extImmutable = this.ext();
				inrImmutable = inr();
			}
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"Base", {}},
		{"C", {
			{"Entry", "function Base.inr()"},
			{"function Base.inr()", "function free()"},
		}},
		{"D", {
			{"Entry", "constructor of D"},
			{"constructor of D", "function Base.inr()"},
			{"function Base.inr()", "function free()"},
		}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"Base", {
			{"Entry", "function Base.ext()"},
			{"function Base.ext()", "function free()"},
		}},
		{"C", {
			{"Entry", "function Base.ext()"},
			{"function Base.ext()", "function free()"},
		}},
		{"D", {
			{"Entry", "function Base.ext()"},
			{"function Base.ext()", "function free()"},
		}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_CASE(function_selector_access)
{
	std::unique_ptr<CompilerStack> compilerStack = parseAndAnalyzeContracts(R"(
		function free() pure {}

		bytes4 constant extFreeConst = Base.ext.selector;
		bytes4 constant pubFreeConst = Base.pub.selector;

		contract Base {
			function ext() external pure { free(); extFreeConst; }
			function pub() public pure { free(); pubFreeConst; }
		}

		contract C is Base {
			bytes4 constant extConst = Base.ext.selector;
			bytes4 constant pubConst = Base.pub.selector;
		}

		contract D is Base {
			bytes4 immutable extImmutable = Base.ext.selector;
			bytes4 immutable pubImmutable = Base.pub.selector;
		}

		contract E is Base {
			bytes4 extVar = Base.ext.selector;
			bytes4 pubVar = Base.pub.selector;
		}

		contract F is Base {
			function f() public pure returns (bytes4, bytes4) {
				return (Base.ext.selector, Base.pub.selector);
			}
		}

		library L {
			bytes4 constant extConst = Base.ext.selector;
			bytes4 constant pubConst = Base.pub.selector;
		}
	)"s);
	std::tuple<CallGraphMap, CallGraphMap> graphs = collectGraphs(*compilerStack);

	std::map<std::string, EdgeNames> expectedCreationEdges = {
		{"Base", {}},
		{"C", {}},
		{"D", {
			{"InternalDispatch", "function Base.pub()"},
			{"function Base.pub()", "function free()"},
		}},
		{"E", {
			{"InternalDispatch", "function Base.pub()"},
			{"function Base.pub()", "function free()"},
		}},
		{"F", {}},
		{"L", {}},
	};

	std::map<std::string, EdgeNames> expectedDeployedEdges = {
		{"Base", {
			{"Entry", "function Base.ext()"},
			{"Entry", "function Base.pub()"},
			{"function Base.ext()", "function free()"},
			{"function Base.pub()", "function free()"},
		}},
		{"C", {
			{"Entry", "function Base.ext()"},
			{"Entry", "function Base.pub()"},
			{"function Base.ext()", "function free()"},
			{"function Base.pub()", "function free()"},
		}},
		{"D", {
			{"InternalDispatch", "function Base.pub()"},
			{"Entry", "function Base.ext()"},
			{"Entry", "function Base.pub()"},
			{"function Base.ext()", "function free()"},
			{"function Base.pub()", "function free()"},
		}},
		{"E", {
			{"InternalDispatch", "function Base.pub()"},
			{"Entry", "function Base.ext()"},
			{"Entry", "function Base.pub()"},
			{"function Base.ext()", "function free()"},
			{"function Base.pub()", "function free()"},
		}},
		{"F", {
			{"InternalDispatch", "function Base.pub()"},
			{"Entry", "function Base.ext()"},
			{"Entry", "function Base.pub()"},
			{"Entry", "function F.f()"},
			{"function Base.ext()", "function free()"},
			{"function Base.pub()", "function free()"},
		}},
		{"L", {}},
	};

	checkCallGraphExpectations(std::get<0>(graphs), expectedCreationEdges);
	checkCallGraphExpectations(std::get<1>(graphs), expectedDeployedEdges);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
