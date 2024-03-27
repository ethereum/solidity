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
#include <libyul/backends/evm/Dominator.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/unit_test.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>

using namespace solidity::yul;

namespace solidity::yul::test
{

typedef std::pair<std::string, std::string> Edge;

struct TestVertex {
	std::string name;
	std::vector<std::shared_ptr<TestVertex>> successors;

	bool operator<(TestVertex const& _other) const
	{
		return name < _other.name;
	}
};

struct ForEachVertexSuccessorTest {
	template<typename Callable>
	void operator()(TestVertex _v, Callable&& _callable) const
	{
		for (auto const& w: _v.successors)
			_callable(*w);
	}
};

struct DominatorFinderTest
{
	DominatorFinderTest(
		std::vector<std::string> _vertices,
		std::vector<Edge> _edges,
		std::vector<size_t> _expectedIdom,
		std::map<std::string, size_t> _expectedDFSIndices,
		std::map<size_t, std::vector<size_t>> _expectedDominatorTree
	)
	{
		soltestAssert(!_vertices.empty() && !_edges.empty());

		for (std::string name: _vertices)
			vertices.emplace(name, std::make_shared<TestVertex>(TestVertex{name, {}}));

		soltestAssert(vertices.size() == _vertices.size());

		for (auto const& [from, to]: _edges)
			vertices[from]->successors.push_back(vertices[to]);

		entry = vertices[_vertices[0]];
		numVertices = _vertices.size();
		expectedIdom = std::move(_expectedIdom);
		expectedDFSIndices = std::move(_expectedDFSIndices);
		expectedDominatorTree = std::move(_expectedDominatorTree);
	}

	size_t numVertices = 0;
	std::shared_ptr<TestVertex> entry = nullptr;
	std::map<std::string, std::shared_ptr<TestVertex>> vertices;
	std::vector<size_t> expectedIdom;
	std::map<std::string, size_t> expectedDFSIndices;
	std::map<size_t, std::vector<size_t>> expectedDominatorTree;
};

class DominatorFinderFixture
{
protected:
	std::map<std::string, size_t> toDFSIndices(std::map<TestVertex, size_t> const& _vertexIndices)
	{
		auto convertIndex = [](std::pair<TestVertex, size_t> const& _pair) -> std::pair<std::string, size_t>
		{
			return {_pair.first.name, _pair.second};
		};
		return _vertexIndices | ranges::views::transform(convertIndex) | ranges::to<std::map<std::string, size_t>>;
	}

	std::vector<std::string> dominatorsByVertexName(std::vector<TestVertex const*> const& _dominators)
	{
		return _dominators | ranges::views::transform([](TestVertex const* d){
			return d->name;
		}) | ranges::to<std::vector<std::string>>;
	}
};

typedef DominatorFinder<TestVertex, ForEachVertexSuccessorTest> TestDominatorFinder;

BOOST_AUTO_TEST_SUITE(Dominators)

BOOST_FIXTURE_TEST_CASE(immediate_dominator_1, DominatorFinderFixture)
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
			Edge("H", "F"),
		},
		{0, 0, 1, 1, 3, 1, 2, 6},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"D", 3},
			{"E", 4},
			{"F", 5},
			{"G", 6},
			{"H", 7},
		},
		{
			{0, {1}},
			{1, {2, 3, 5}},
			{2, {6}},
			{3, {4}},
			{6, {7}}
		}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(immediate_dominator_2, DominatorFinderFixture)
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
			Edge("G", "C"),
		},
		{0, 0, 0, 0, 0, 4, 4},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"G", 3},
			{"D", 4},
			{"E", 5},
			{"F", 6},
		},
		{
			{0, {1, 2, 3, 4}},
			{4, {5, 6}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(immediate_dominator_3, DominatorFinderFixture)
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
			Edge("I", "H"),
		},
		{0, 0, 0, 0, 1, 1, 1, 1, 5},
		{
			{"A", 0},
			{"B", 1},
			{"C", 2},
			{"D", 3},
			{"I", 4},
			{"E", 5},
			{"H", 6},
			{"G", 7},
			{"F", 8},
		},
		{
			{0, {1, 2, 3}},
			{1, {4, 5, 6, 7}},
			{5, {8}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(langauer_tarjan_p122_fig1, DominatorFinderFixture)
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
			Edge("I", "K"),
		},
		{0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 9, 9, 11},
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
			{"J", 12},
		},
		{
			{0, {1, 2, 3, 5, 6, 7, 8, 9}},
			{3, {4}},
			{9, {10, 11}},
			{11, {12}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(loukas_georgiadis, DominatorFinderFixture)
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
			Edge("X7", "X6"),
		},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
			{"Y", 9},
		},
		{
			{0, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(itworst, DominatorFinderFixture)
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
			Edge("Z3", "Z2"),
		},
		{0, 0, 0, 0, 0, 4, 5, 6, 7, 8, 0, 0, 0},
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
			{"Z3", 12},
		},
		{
			{0, {1, 2, 3, 4, 10, 11, 12}},
			{4, {5}},
			{5, {6}},
			{6, {7}},
			{7, {8}},
			{8, {9}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(idfsquad, DominatorFinderFixture)
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
			Edge("Z3", "Y3"),
		},
		{0, 0, 0, 0, 0, 0, 0, 0, 1, 8},
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
			{"X3", 9},
		},
		{
			{0, {1, 2, 3, 4, 5, 6, 7}},
			{1, {8}},
			{8, {9}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(ibsfquad, DominatorFinderFixture)
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
			Edge("X2", "X1"),
		},
		{0, 0, 0, 0, 0, 0, 5},
		{
			{"R", 0},
			{"W", 1},
			{"X1", 2},
			{"X2", 3},
			{"X3", 4},
			{"Y", 5},
			{"Z", 6},
		},
		{
			{0, {1, 2, 3, 4, 5}},
			{5, {6}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(sncaworst, DominatorFinderFixture)
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
			Edge("X3", "Y3"),
		},
		{0, 0, 1, 2, 0, 0, 0},
		{
			{"R", 0},
			{"X1", 1},
			{"X2", 2},
			{"X3", 3},
			{"Y1", 4},
			{"Y2", 5},
			{"Y3", 6},
		},
		{
			{0, {1, 4, 5, 6}},
			{1, {2}},
			{2, {3}}
		}
	);
	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	BOOST_TEST(dominatorFinder.immediateDominators() == test.expectedIdom);
	BOOST_TEST(dominatorFinder.dominatorTree() == test.expectedDominatorTree);
}

BOOST_FIXTURE_TEST_CASE(collect_all_dominators_of_a_vertex, DominatorFinderFixture)
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
			Edge("H", "F"),
		},
		{},
		{},
		{}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["A"])) == std::vector<std::string>());
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["B"])) == std::vector<std::string>({"A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["C"])) == std::vector<std::string>({"B", "A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["D"])) == std::vector<std::string>({"B", "A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["E"])) == std::vector<std::string>({"D", "B", "A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["F"])) == std::vector<std::string>({"B", "A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["G"])) == std::vector<std::string>({"C", "B", "A"}));
	BOOST_TEST(dominatorsByVertexName(dominatorFinder.dominatorsOf(*test.vertices["H"])) == std::vector<std::string>({"G", "C", "B", "A"}));
}

BOOST_FIXTURE_TEST_CASE(dominators_of_a_non_existent_vertex, DominatorFinderFixture)
{
	DominatorFinderTest test(
		{"A", "B", "C", "D"},
		{
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("B", "D"),
		},
		{},
		{},
		{}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_CHECK_THROW(dominatorsByVertexName(dominatorFinder.dominatorsOf(TestVertex{"Z", {}})), util::ElementNotFound);
}

BOOST_FIXTURE_TEST_CASE(check_dominance, DominatorFinderFixture)
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
			Edge("H", "F"),
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
			{"H", 7},
		},
		{}
	);

	auto makeDominanceVertexRelation = [&](std::vector<size_t> _indices = {}){
		std::vector<bool> dominance(test.numVertices, false);
		for (auto i: _indices)
		{
			soltestAssert(i < test.numVertices);
			dominance[i] = true;
		}
		return dominance;
	};

	// Dominance truth table for all vertices.
	// Note that it includes self-dominance relation.
	std::vector<std::vector<bool>> expectedDominanceByVertex = {
		makeDominanceVertexRelation(ranges::views::iota(0u, static_cast<unsigned>(test.numVertices)) | ranges::to<std::vector<size_t>>), // A dominates all vertices, including itself
		makeDominanceVertexRelation(ranges::views::iota(1u, static_cast<unsigned>(test.numVertices)) | ranges::to<std::vector<size_t>>), // B, C, D, E, F, G, H
		makeDominanceVertexRelation({2, 6, 7}), // C, G, H
		makeDominanceVertexRelation({3, 4}),    // D, E
		makeDominanceVertexRelation({4}),       // E
		makeDominanceVertexRelation({5}),       // F
		makeDominanceVertexRelation({6, 7}),    // G, H
		makeDominanceVertexRelation({7})        // H
	};

	soltestAssert(expectedDominanceByVertex.size() == test.numVertices);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);

	// Asserts that the indices are computed correctly, since we need a reverse map from indices to vertex name
	// to retrieve the respective vertices in the test below.
	BOOST_TEST(toDFSIndices(dominatorFinder.vertexIndices()) == test.expectedDFSIndices);
	std::map<size_t, std::string> reverseDFSIndicesMap = test.expectedDFSIndices | ranges::views::transform([](auto const& pair){
		return std::make_pair(pair.second, pair.first);
	}) | ranges::to<std::map<size_t, std::string>>;

	for (size_t i = 0; i < expectedDominanceByVertex.size(); ++i)
	{
		soltestAssert(expectedDominanceByVertex[i].size() == test.numVertices);
		for (size_t j = 0; j < expectedDominanceByVertex[i].size(); ++j)
		{
			bool result = dominatorFinder.dominates(*test.vertices[reverseDFSIndicesMap[i]], *test.vertices[reverseDFSIndicesMap[j]]);
			BOOST_CHECK_MESSAGE(
				result == expectedDominanceByVertex[i][j],
				"Vertex: " + reverseDFSIndicesMap[i] + " (" + std::to_string(i) + ") expected to" +
				(expectedDominanceByVertex[i][j] ? "" : " not") +
				" dominates vertex " + reverseDFSIndicesMap[j] + " (" + std::to_string(j) + ") but returned: " +
				std::to_string(result)
            );
		}
	}
}

BOOST_FIXTURE_TEST_CASE(check_dominance_of_non_existent_vertex, DominatorFinderFixture)
{
	DominatorFinderTest test(
		{"A", "B", "C", "D"},
		{
			Edge("A", "B"),
			Edge("B", "C"),
			Edge("B", "D"),
		},
		{},
		{},
		{}
	);

	TestDominatorFinder dominatorFinder(*test.entry, test.numVertices);
	BOOST_CHECK_THROW(dominatorFinder.dominates(TestVertex{"Z", {}}, *test.vertices["A"]), util::ElementNotFound);
}



BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::yul::test
