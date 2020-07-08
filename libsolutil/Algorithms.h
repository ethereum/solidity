// SPDX-License-Identifier: GPL-3.0
#pragma once


#include <functional>
#include <set>

namespace solidity::util
{

/**
 * Detector for cycles in directed graphs. It returns the first
 * vertex on the path towards a cycle or a nullptr if there is
 * no reachable cycle starting from a given vertex.
 */
template <typename V>
class CycleDetector
{
public:
	using Visitor = std::function<void(V const&, CycleDetector&, size_t)>;

	/// Initializes the cycle detector
	/// @param _visit function that is given the current vertex
	///               and is supposed to call @a run on all
	///               adjacent vertices.
	explicit CycleDetector(Visitor _visit):
		m_visit(std::move(_visit))
	{  }

	/// Recursively perform cycle detection starting
	/// (or continuing) with @param _vertex
	/// @returns the first vertex on the path towards a cycle from @a _vertex
	/// or nullptr if no cycle is reachable from @a _vertex.
	V const* run(V const& _vertex)
	{
		if (m_firstCycleVertex)
			return m_firstCycleVertex;
		if (m_processed.count(&_vertex))
			return nullptr;
		else if (m_processing.count(&_vertex))
			return m_firstCycleVertex = &_vertex;
		m_processing.insert(&_vertex);

		m_depth++;
		m_visit(_vertex, *this, m_depth);
		m_depth--;
		if (m_firstCycleVertex && m_depth == 1)
			m_firstCycleVertex = &_vertex;

		m_processing.erase(&_vertex);
		m_processed.insert(&_vertex);
		return m_firstCycleVertex;
	}

private:
	Visitor m_visit;
	std::set<V const*> m_processing;
	std::set<V const*> m_processed;
	size_t m_depth = 0;
	V const* m_firstCycleVertex = nullptr;
};

/**
 * Generic breadth first search.
 *
 * Note that V needs to be a comparable value type. If it is not, use a pointer type,
 * but note that this might lead to non-deterministic traversal.
 *
 * Example: Gather all (recursive) children in a graph starting at (and including) ``root``:
 *
 * Node const* root = ...;
 * std::set<Node const*> allNodes = BreadthFirstSearch<Node const*>{{root}}.run([](Node const* _node, auto&& _addChild) {
 *   // Potentially process ``_node``.
 *   for (Node const& _child: _node->children())
 *     // Potentially filter the children to be visited.
 *     _addChild(&_child);
 * }).visited;
 */
template<typename V>
struct BreadthFirstSearch
{
	/// Runs the breadth first search. The verticesToTraverse member of the struct needs to be initialized.
	/// @param _forEachChild is a callable of the form [...](V const& _node, auto&& _addChild) { ... }
	/// that is called for each visited node and is supposed to call _addChild(childNode) for every child
	/// node of _node.
	template<typename ForEachChild>
	BreadthFirstSearch& run(ForEachChild&& _forEachChild)
	{
		while (!verticesToTraverse.empty())
		{
			V v = *verticesToTraverse.begin();
			verticesToTraverse.erase(verticesToTraverse.begin());
			visited.insert(v);

			_forEachChild(v, [this](V _vertex) {
				if (!visited.count(_vertex))
					verticesToTraverse.emplace(std::move(_vertex));
			});
		}
		return *this;
	}
	void abort()
	{
		verticesToTraverse.clear();
	}

	std::set<V> verticesToTraverse;
	std::set<V> visited{};
};

}
