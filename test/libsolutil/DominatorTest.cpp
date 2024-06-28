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
 * Unit tests for the algorithm to find dominators from a graph.
 */
#include <libsolutil/Dominator.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/unit_test.hpp>

#include <fmt/format.h>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view.hpp>

using namespace solidity::util;

namespace solidity::util::test
{

typedef size_t TestVertexId;
typedef std::string TestVertexLabel;
typedef std::pair<TestVertexLabel, TestVertexLabel> Edge;

struct TestVertex {
	using Id = TestVertexId;

	TestVertexId id;
	TestVertexLabel label;
	std::vector<TestVertex const*> successors;
};

struct ForEachVertexSuccessorTest {
	template<typename Callable>
	void operator()(TestVertex const& _v, Callable&& _callable) const
	{
		for (auto const& w: _v.successors)
			_callable(*w);
	}
};

typedef DominatorFinder<TestVertex, ForEachVertexSuccessorTest> TestDominatorFinder;

class DominatorFinderTest
{
public:
	DominatorFinderTest(
		std::vector<TestVertexLabel> const& _vertices,
		std::vector<Edge> const& _edges,
		std::map<TestVertexLabel, TestVertexLabel> const& _expectedIdom,
		std::map<TestVertexLabel, size_t> const& _expectedDFSIndices,
		std::map<TestVertexLabel, std::set<TestVertexLabel>> const& _expectedDominatorTree
	)
	{
		soltestAssert(!_vertices.empty());

		// NOTE: We use the indices of the vertices in the ``_vertices`` vector as vertices ids.
		TestVertexId id = 0;
		vertices = _vertices | ranges::views::transform([&](TestVertexLabel const& label) -> TestVertex {
			verticesIdMap[label] = id;
			return {id++, label, {}};
		}) | ranges::to<std::vector<TestVertex>>;
		soltestAssert(vertices.size() == _vertices.size());
		soltestAssert(verticesIdMap.size() == _vertices.size());
		soltestAssert(id == _vertices.size());

		// Populate the successors of the vertices.
		for (auto const& [from, to]: _edges)
			vertices[verticesIdMap[from]].successors.emplace_back(&vertices[verticesIdMap[to]]);

		entry = &vertices[0];
		numVertices = _vertices.size();
		expectedIdom = labelToId(_expectedIdom);
		expectedDFSIndices = labelMapToVertexId(_expectedDFSIndices);
		expectedDominatorTree = dominatorTreeByVertexId(_expectedDominatorTree);
	}

	// Converts a vector of vertex labels to vector of vertex Ids
	// NOTE: Only used for testing purposes.
	std::map<TestVertexId, TestVertexId> labelToId(std::map<TestVertexLabel, TestVertexLabel> const& _vertexLabels) const
	{
		std::map<TestVertexId, TestVertexId> idom;
		for (auto const& [vlabel, idomLabel]: _vertexLabels) {
			soltestAssert(verticesIdMap.count(vlabel) && verticesIdMap.count(idomLabel));
			idom[verticesIdMap.at(vlabel)] = verticesIdMap.at(idomLabel);
		}
		return idom;
	}

	// Converts a map of vertex label to DFS indices to a map of vertex Ids to DFS indices.
	// NOTE: Only used for testing purposes.
	std::map<TestVertexId, size_t> labelMapToVertexId(std::map<TestVertexLabel, size_t> const& _dfsIndices) const
	{
		std::map<TestVertexId, size_t> dfsIndices;
		for (auto const& [label, index] : _dfsIndices) {
			soltestAssert(verticesIdMap.count(label));
			dfsIndices[verticesIdMap.at(label)] = index;
		}
		return dfsIndices;
	}

	// Converts a vector of vertex IDs to a vector of vertex labels.
	// NOTE: Only used for testing purposes.
	std::vector<TestVertexLabel> dominatorsByVertexLabel(std::vector<TestVertexId> const& _dominators) const
	{
		return _dominators | ranges::views::transform([&](TestVertexId id){
			return vertices[id].label;
		}) | ranges::to<std::vector<TestVertexLabel>>;
	}

	// Converts a map of vertex label to set of labels back to a Dominator tree, i.e. map of vertex IDs to sets of vertex IDs
	// NOTE: Only used for testing purposes.
	std::map<TestVertexId, std::set<TestVertexId>> dominatorTreeByVertexId(std::map<TestVertexLabel, std::set<TestVertexLabel>> const& _domTreeByLabel) const
	{
		auto labelSetToIdSet = [&](std::set<TestVertexLabel> const& labels) -> std::set<TestVertexId> {
			return labels
				| ranges::views::transform([&](TestVertexLabel const& label) {
					soltestAssert(verticesIdMap.count(label));
					return verticesIdMap.at(label);
				})
				| ranges::to<std::set<TestVertexId>>;
		};

		std::map<TestVertexId, std::set<TestVertexId>> domTree;
		for (auto const& [label, dominators] : _domTreeByLabel) {
			soltestAssert(verticesIdMap.count(label));
			domTree[verticesIdMap.at(label)] = labelSetToIdSet(dominators);
		}

		return domTree;
	}

	size_t numVertices = 0;
	TestVertex const* entry = nullptr;
	std::vector<TestVertex> vertices;
	// Reverse map from vertices labels to ids
	std::map<TestVertexLabel, TestVertexId> verticesIdMap;
	std::map<TestVertexId, TestVertexId> expectedIdom;
	std::map<TestVertexId, size_t> expectedDFSIndices;
	std::map<TestVertexId, std::set<TestVertexId>> expectedDominatorTree;
};

BOOST_AUTO_TEST_SUITE(Dominators)

BOOST_AUTO_TEST_CASE(immediate_dominator_1)
{
	//            A
	//            │
	//            ▼
	//        ┌───B
	//        │   │
	//        ▼   │
	//        C ──┼───┐
	//        │   │   │
	//        ▼   │   ▼
	//        D◄──┘   G
	//        │       │
	//        ▼       ▼
	//        E       H
	//        │       │
	//        └──►F◄──┘
	DominatorFinderTest test(
		{"A", "B", "C", "D", "E", "F", "G", "H"}, // Vertices
		{ // Edges
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("B", "D"),
			Edge("C", "D"),
			Edge("C", "G"),
			Edge("D", "E"),
			Edge("E", "F"),
			Edge("G", "H"),
			Edge("H", "F")
		},
		{ // Immediate dominators
			{"A", "A"},
			{"B", "A"},
			{"C", "B"},
			{"D", "B"},
			{"E", "D"},
			{"F", "B"},
			{"G", "C"},
			{"H", "G"}
		},
		{ // DFS indices
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"D", 3},
			{"E", 4},
			{"F", 5},
			{"G", 6},
			{"H", 7}
		},
		{ // Dominator tree
			{"A", {"B"}},
			{"B", {"C", "D", "F"}},
			{"C", {"G"}},
			{"D", {"E"}},
			{"G", {"H"}}
		}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(immediate_dominator_2)
{
	//    ┌────►A──────┐
	//    │     │      ▼
	//    │ B◄──┘   ┌──D──┐
	//    │ │       │     │
	//    │ ▼       ▼     ▼
	//    └─C◄───┐  E     F
	//      │    │  │     │
	//      └───►G◄─┴─────┘
	DominatorFinderTest test(
		{"A", "B", "C", "D", "E", "F", "G"},
		{
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("C", "G"),
			Edge("C", "A"),
			Edge("A", "D"),
			Edge("D", "E"),
			Edge("D", "F"),
			Edge("E", "G"),
			Edge("F", "G"),
			Edge("G", "C")
		},
		{
			{"A", "A"},
			{"B", "A"},
			{"C", "A"},
			{"D", "A"},
			{"E", "D"},
			{"F", "D"},
			{"G", "A"}
		},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"G", 3},
			{"D", 4},
			{"E", 5},
			{"F", 6}
		},
		{
			{"A", {"B", "C", "D", "G"}},
			{"D", {"E", "F"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(immediate_dominator_3)
{
	//    ┌─────────┐
	//    │         ▼
	//    │     ┌───A───┐
	//    │     │       │
	//    │     ▼       ▼
	//    │ ┌──►C◄───── B──┬──────┐
	//    │ │   │       ▲  │      │
	//    │ │   │  ┌────┘  │      │
	//    │ │   ▼  │       ▼      ▼
	//    │ │   D──┘  ┌───►E◄─────I
	//    │ │   ▲     │    │      │
	//    │ │   │     │    ├───┐  │
	//    │ │   │     │    │   │  │
	//    │ │   │     │    ▼   │  ▼
	//    │ └───┼─────┼────F   └─►H
	//    │     │     │    │      │
	//    │     │     │    │      │
	//    │     │     │    │      │
	//    │     └─────┴─G◄─┴──────┘
	//    │             │
	//    └─────────────┘
	DominatorFinderTest test(
		{"A", "B", "C", "D", "E", "F", "G", "H", "I"},
		{
			Edge("A", "B"),
			Edge("A", "C"),
			Edge("B", "C"),
			Edge("B", "I"),
			Edge("B", "E"),
			Edge("C", "D"),
			Edge("D", "B"),
			Edge("E", "H"),
			Edge("E", "F"),
			Edge("F", "G"),
			Edge("F", "C"),
			Edge("G", "E"),
			Edge("G", "A"),
			Edge("G", "D"),
			Edge("H", "G"),
			Edge("I", "E"),
			Edge("I", "H")
		},
		{
			{"A", "A"},
			{"B", "A"},
			{"C", "A"},
			{"D", "A"},
			{"E", "B"},
			{"F", "E"},
			{"G", "B"},
			{"H", "B"},
			{"I", "B"}
		},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"D", 3},
			{"I", 4},
			{"E", 5},
			{"H", 6},
			{"G", 7},
			{"F", 8}
		},
		{
			{"A", {"B", "C", "D"}},
			{"B", {"I", "E", "H", "G"}},
			{"E", {"F"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(langauer_tarjan_p122_fig1)
{
	// T. Lengauer and R. E. Tarjan pg. 122 fig. 1
	// ref: https://www.cs.princeton.edu/courses/archive/spr03/cs423/download/dominators.pdf
	DominatorFinderTest test(
		{"R", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "L", "K"},
		{
			Edge("R", "B"),
			Edge("R", "A"),
			Edge("R", "C"),
			Edge("B", "A"),
			Edge("B", "D"),
			Edge("B", "E"),
			Edge("A", "D"),
			Edge("D", "L"),
			Edge("L", "H"),
			Edge("E", "H"),
			Edge("H", "E"),
			Edge("H", "K"),
			Edge("K", "I"),
			Edge("K", "R"),
			Edge("C", "F"),
			Edge("C", "G"),
			Edge("F", "I"),
			Edge("G", "I"),
			Edge("G", "J"),
			Edge("J", "I"),
			Edge("I", "K")
		},
		{
			{"R", "R"},
			{"A", "R"},
			{"B", "R"},
			{"C", "R"},
			{"D", "R"},
			{"E", "R"},
			{"F", "C"},
			{"G", "C"},
			{"H", "R"},
			{"I", "R"},
			{"J", "G"},
			{"L", "D"},
			{"K", "R"}
		},
		{
			{"R", 0},
			{"B", 1},
			{"A", 2},
			{"D", 3},
			{"L", 4},
			{"H", 5},
			{"E", 6},
			{"K", 7},
			{"I", 8},
			{"C", 9},
			{"F", 10},
			{"G", 11},
			{"J", 12}
		},
		{
			{"R", {"B", "A", "D", "H", "E", "K", "I", "C"}},
			{"D", {"L"}},
			{"C", {"F", "G"}},
			{"G", {"J"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(loukas_georgiadis)
{
	// Extracted from Loukas Georgiadis Dissertation - Linear-Time Algorithms for Dominators and Related Problems
	// pg. 12 Fig. 2.2
	// ref: https://www.cs.princeton.edu/techreports/2005/737.pdf
	DominatorFinderTest test(
		{"R", "W", "X1", "X2", "X3", "X4", "X5", "X6", "X7", "Y"},
		{
			Edge("R", "W"),
			Edge("R", "Y"),
			Edge("W", "X1"),
			Edge("Y", "X7"),
			Edge("X1", "X2"),
			Edge("X2", "X1"),
			Edge("X2", "X3"),
			Edge("X3", "X2"),
			Edge("X3", "X4"),
			Edge("X4", "X3"),
			Edge("X4", "X5"),
			Edge("X5", "X4"),
			Edge("X5", "X6"),
			Edge("X6", "X5"),
			Edge("X6", "X7"),
			Edge("X7", "X6")
		},
		{
			{"R", "R"},
			{"W", "R"},
			{"X1", "R"},
			{"X2", "R"},
			{"X3", "R"},
			{"X4", "R"},
			{"X5", "R"},
			{"X6", "R"},
			{"X7", "R"},
			{"Y", "R"}
		},
		{
			{"R", 0},
			{"W", 1},
			{"X1", 2},
			{"X2", 3},
			{"X3", 4},
			{"X4", 5},
			{"X5", 6},
			{"X6", 7},
			{"X7", 8},
			{"Y", 9}
		},
		{
			{"R", {"W", "X1", "X2", "X3", "X4", "X5", "X6", "X7", "Y"}},
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(itworst)
{
	// Worst-case families for k = 3
	// Example itworst(3) pg. 26 fig. 2.9
	// ref: https://www.cs.princeton.edu/techreports/2005/737.pdf
	DominatorFinderTest test(
		{"R", "W1", "W2", "W3", "X1", "X2", "X3", "Y1", "Y2", "Y3", "Z1", "Z2", "Z3"},
		{
			Edge("R", "W1"),
			Edge("R", "X1"),
			Edge("R", "Z3"),
			Edge("W1", "W2"),
			Edge("W2", "W3"),
			Edge("X1", "X2"),
			Edge("X2", "X3"),
			Edge("X3", "Y1"),
			Edge("Y1", "W1"),
			Edge("Y1", "W2"),
			Edge("Y1", "W3"),
			Edge("Y1", "Y2"),
			Edge("Y2", "W1"),
			Edge("Y2", "W2"),
			Edge("Y2", "W3"),
			Edge("Y2", "Y3"),
			Edge("Y3", "W1"),
			Edge("Y3", "W2"),
			Edge("Y3", "W3"),
			Edge("Y3", "Z1"),
			Edge("Z1", "Z2"),
			Edge("Z2", "Z1"),
			Edge("Z2", "Z3"),
			Edge("Z3", "Z2")
		},
		{
			{"R", "R"},
			{"W1", "R"},
			{"W2", "R"},
			{"W3", "R"},
			{"X1", "R"},
			{"X2", "X1"},
			{"X3", "X2"},
			{"Y1", "X3"},
			{"Y2", "Y1"},
			{"Y3", "Y2"},
			{"Z1", "R"},
			{"Z2", "R"},
			{"Z3", "R"}
		},
		{
			{"R", 0},
			{"W1", 1},
			{"W2", 2},
			{"W3", 3},
			{"X1", 4},
			{"X2", 5},
			{"X3", 6},
			{"Y1", 7},
			{"Y2", 8},
			{"Y3", 9},
			{"Z1", 10},
			{"Z2", 11},
			{"Z3", 12}
		},
		{
			{"R", {"W1", "W2", "W3", "X1", "Z1", "Z2", "Z3"}},
			{"X1", {"X2"}},
			{"X2", {"X3"}},
			{"X3", {"Y1"}},
			{"Y1", {"Y2"}},
			{"Y2", {"Y3"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(idfsquad)
{
	// Worst-case families for k = 3
	// Example idfsquad(3) pg. 26 fig. 2.9
	// ref: https://www.cs.princeton.edu/techreports/2005/737.pdf
	DominatorFinderTest test(
		{"R", "X1", "X2", "X3", "Y1", "Y2", "Y3", "Z1", "Z2", "Z3"},
		{
			Edge("R", "X1"),
			Edge("R", "Z1"),
			Edge("X1", "Y1"),
			Edge("X1", "X2"),
			Edge("X2", "X3"),
			Edge("X2", "Y2"),
			Edge("X3", "Y3"),
			Edge("Y1", "Z1"),
			Edge("Y1", "Z2"),
			Edge("Z1", "Y1"),
			Edge("Y2", "Z2"),
			Edge("Y2", "Z3"),
			Edge("Z2", "Y2"),
			Edge("Y3", "Z3"),
			Edge("Z3", "Y3")
		},
		{
			{"R", "R"},
			{"X1", "R"},
			{"X2", "X1"},
			{"X3", "X2"},
			{"Y1", "R"},
			{"Y2", "R"},
			{"Y3", "R"},
			{"Z1", "R"},
			{"Z2", "R"},
			{"Z3", "R"}
		},
		{
			{"R", 0},
			{"X1", 1},
			{"Y1", 2},
			{"Z1", 3},
			{"Z2", 4},
			{"Y2", 5},
			{"Z3", 6},
			{"Y3", 7},
			{"X2", 8},
			{"X3", 9}
		},
		{
			{"R", {"X1", "Y1", "Z1", "Z2", "Y2", "Z3", "Y3"}},
			{"X1", {"X2"}},
			{"X2", {"X3"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(ibsfquad)
{
	// Worst-case families for k = 3
	// Example ibfsquad(3) pg. 26 fig. 2.9
	// ref: https://www.cs.princeton.edu/techreports/2005/737.pdf
	DominatorFinderTest test(
		{"R", "W", "X1", "X2", "X3", "Y", "Z"},
		{
			Edge("R", "W"),
			Edge("R", "Y"),
			Edge("W", "X1"),
			Edge("W", "X2"),
			Edge("W", "X3"),
			Edge("Y", "Z"),
			Edge("Z", "X3"),
			Edge("X3", "X2"),
			Edge("X2", "X1")
		},
		{
			{"R", "R"},
			{"W", "R"},
			{"X1", "R"},
			{"X2", "R"},
			{"X3", "R"},
			{"Y", "R"},
			{"Z", "Y"}
		},
		{
			{"R", 0},
			{"W", 1},
			{"X1", 2},
			{"X2", 3},
			{"X3", 4},
			{"Y", 5},
			{"Z", 6}
		},
		{
			{"R", {"W", "X1", "X2", "X3", "Y"}},
			{"Y", {"Z"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(sncaworst)
{
	// Worst-case families for k = 3
	// Example sncaworst(3) pg. 26 fig. 2.9
	// ref: https://www.cs.princeton.edu/techreports/2005/737.pdf
	DominatorFinderTest test(
		{"R", "X1", "X2", "X3", "Y1", "Y2", "Y3"},
		{
			Edge("R", "X1"),
			Edge("R", "Y1"),
			Edge("R", "Y2"),
			Edge("R", "Y3"),
			Edge("X1", "X2"),
			Edge("X2", "X3"),
			Edge("X3", "Y1"),
			Edge("X3", "Y2"),
			Edge("X3", "Y3")
		},
		{
			{"R", "R"},
			{"X1", "R"},
			{"X2", "X1"},
			{"X3", "X2"},
			{"Y1", "R"},
			{"Y2", "R"},
			{"Y3", "R"}
		},
		{
			{"R", 0},
			{"X1", 1},
			{"X2", 2},
			{"X3", 3},
			{"Y1", 4},
			{"Y2", 5},
			{"Y3", 6}
		},
		{
			{"R", {"X1", "Y1", "Y2", "Y3"}},
			{"X1", {"X2"}},
			{"X2", {"X3"}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_CASE(collect_all_dominators_of_a_vertex)
{
	//            A
	//            │
	//            ▼
	//        ┌───B
	//        │   │
	//        ▼   │
	//        C ──┼───┐
	//        │   │   │
	//        ▼   │   ▼
	//        D◄──┘   G
	//        │       │
	//        ▼       ▼
	//        E       H
	//        │       │
	//        └──►F◄──┘
	DominatorFinderTest test(
		{"A", "B", "C", "D", "E", "F", "G", "H"},
		{
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("B", "D"),
			Edge("C", "D"),
			Edge("C", "G"),
			Edge("D", "E"),
			Edge("E", "F"),
			Edge("G", "H"),
			Edge("H", "F")
		},
		{},
		{},
		{}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["A"])) == std::vector<TestVertexLabel>());
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["B"])) == std::vector<TestVertexLabel>({"A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["C"])) == std::vector<TestVertexLabel>({"B", "A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["D"])) == std::vector<TestVertexLabel>({"B", "A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["E"])) == std::vector<TestVertexLabel>({"D", "B", "A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["F"])) == std::vector<TestVertexLabel>({"B", "A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["G"])) == std::vector<TestVertexLabel>({"C", "B", "A"}));
	BOOST_TEST(test.dominatorsByVertexLabel(dominatorFinder.dominatorsOf(test.verticesIdMap["H"])) == std::vector<TestVertexLabel>({"G", "C", "B", "A"}));
}

BOOST_AUTO_TEST_CASE(check_dominance)
{
	//            A
	//            │
	//            ▼
	//        ┌───B
	//        │   │
	//        ▼   │
	//        C ──┼───┐
	//        │   │   │
	//        ▼   │   ▼
	//        D◄──┘   G
	//        │       │
	//        ▼       ▼
	//        E       H
	//        │       │
	//        └──►F◄──┘
	DominatorFinderTest test(
		{"A", "B", "C", "D", "E", "F", "G", "H"},
		{
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("B", "D"),
			Edge("C", "D"),
			Edge("C", "G"),
			Edge("D", "E"),
			Edge("E", "F"),
			Edge("G", "H"),
			Edge("H", "F")
		},
		{},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"D", 3},
			{"E", 4},
			{"F", 5},
			{"G", 6},
			{"H", 7}
		},
		{}
	);

	// Helper function to create a dominance relation for a vertex with all other vertices.
	//
	// @param _indices: The indices of the vertices that the vertex dominates.
	// @return: A vector of booleans where the index represents the vertex and the value
	// represents if the vertex dominates the other vertex at that index.
	auto makeDominanceVertexRelation = [&](std::vector<size_t> const& _indices = {}){
		std::vector<bool> dominance(test.numVertices, false);
		for (size_t i: _indices)
		{
			soltestAssert(i < test.numVertices);
			dominance[i] = true;
		}
		return dominance;
	};

	// Dominance truth table for all vertices.
	// Note that it includes self-dominance relation.
	std::vector<std::vector<bool>> expectedDominanceTable = {
		std::vector<bool>(test.numVertices, true),    // A dominates all vertices, including itself
		makeDominanceVertexRelation({1,2,3,4,5,6,7}), // B, C, D, E, F, G, H
		makeDominanceVertexRelation({2, 6, 7}), // C, G, H
		makeDominanceVertexRelation({3, 4}),    // D, E
		makeDominanceVertexRelation({4}),       // E
		makeDominanceVertexRelation({5}),       // F
		makeDominanceVertexRelation({6, 7}),    // G, H
		makeDominanceVertexRelation({7})        // H
	};
	soltestAssert(expectedDominanceTable.size() == test.numVertices);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);

	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	// Check if the dominance table is as expected.
	for (size_t i = 0; i < expectedDominanceTable.size(); ++i)
	{
		soltestAssert(expectedDominanceTable[i].size() == test.numVertices);
		for (size_t j = 0; j < expectedDominanceTable[i].size(); ++j)
		{
			bool iDominatesJ = dominatorFinder.dominates(i, j);
			BOOST_CHECK_MESSAGE(
				iDominatesJ == expectedDominanceTable[i][j],
				fmt::format(
					"Vertex: {} expected to {} dominate vertex {} but returned: {}\n",
					i,
					(iDominatesJ ? "" : "not"),
					j,
					iDominatesJ
				)
			);
		}
	}
}

BOOST_AUTO_TEST_CASE(no_edges)
{
	DominatorFinderTest test(
		{"A"},
		{},
		{
			{"A", "A"},
		},
		{
			{"A", 0},
		},
		{}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorFinder.idom() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dfsIndexById() == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::util::test
