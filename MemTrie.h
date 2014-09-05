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
/** @file MemTrie.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

namespace dev
{

class MemTrieNode;

/**
 * @brief Merkle Patricia Tree "Trie": a modifed base-16 Radix tree.
 */
class MemTrie
{
public:
	MemTrie(): m_root(nullptr) {}
	~MemTrie();

	h256 hash256() const;
	bytes rlp() const;

	void debugPrint();

	std::string const& at(std::string const& _key) const;
	void insert(std::string const& _key, std::string const& _value);
	void remove(std::string const& _key);

private:
	MemTrieNode* m_root;
};

}
