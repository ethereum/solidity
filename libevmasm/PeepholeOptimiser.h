// SPDX-License-Identifier: GPL-3.0
/**
 * @file PeepholeOptimiser.h
 * Performs local optimising code changes to assembly.
 */
#pragma once

#include <vector>
#include <cstddef>
#include <iterator>

namespace solidity::evmasm
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;

class PeepholeOptimisationMethod
{
public:
	virtual ~PeepholeOptimisationMethod() = default;
	virtual size_t windowSize() const;
	virtual bool apply(AssemblyItems::const_iterator _in, std::back_insert_iterator<AssemblyItems> _out);
};

class PeepholeOptimiser
{
public:
	explicit PeepholeOptimiser(AssemblyItems& _items): m_items(_items) {}
	virtual ~PeepholeOptimiser() = default;

	bool optimise();

private:
	AssemblyItems& m_items;
	AssemblyItems m_optimisedItems;
};

}
