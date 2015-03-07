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
/** @file MemTrie.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "MemTrie.h"

#include <libdevcrypto/TrieCommon.h>
#include <libdevcrypto/SHA3.h>
#include <libethcore/Common.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

namespace dev
{

#define ENABLE_DEBUG_PRINT 0

/*/
#define APPEND_CHILD appendData
/*/
#define APPEND_CHILD appendRaw
/**/

class MemTrieNode
{
public:
	MemTrieNode() {}
	virtual ~MemTrieNode() {}

	virtual std::string const& at(bytesConstRef _key) const = 0;
	virtual MemTrieNode* insert(bytesConstRef _key, std::string const& _value) = 0;
	virtual MemTrieNode* remove(bytesConstRef _key) = 0;
	void putRLP(RLPStream& _parentStream) const;

#if ENABLE_DEBUG_PRINT
	void debugPrint(std::string const& _indent = "") const { std::cerr << std::hex << hash256() << ":" << std::dec << std::endl; debugPrintBody(_indent); }
#endif

	/// 256-bit hash of the node - this is a SHA-3/256 hash of the RLP of the node.
	h256 hash256() const { RLPStream s; makeRLP(s); return dev::sha3(s.out()); }
	bytes rlp() const { RLPStream s; makeRLP(s); return s.out(); }
	void mark() { m_hash256 = h256(); }

protected:
	virtual void makeRLP(RLPStream& _intoStream) const = 0;

#if ENABLE_DEBUG_PRINT
	virtual void debugPrintBody(std::string const& _indent = "") const = 0;
#endif

	static MemTrieNode* newBranch(bytesConstRef _k1, std::string const& _v1, bytesConstRef _k2, std::string const& _v2);

private:
	mutable h256 m_hash256;
};

static const std::string c_nullString;

class TrieExtNode: public MemTrieNode
{
public:
	TrieExtNode(bytesConstRef _bytes): m_ext(_bytes.begin(), _bytes.end()) {}

	bytes m_ext;
};

class TrieBranchNode: public MemTrieNode
{
public:
	TrieBranchNode(std::string const& _value): m_value(_value)
	{
		memset(m_nodes.data(), 0, sizeof(MemTrieNode*) * 16);
	}

	TrieBranchNode(byte _i1, MemTrieNode* _n1, std::string const& _value = std::string()): m_value(_value)
	{
		memset(m_nodes.data(), 0, sizeof(MemTrieNode*) * 16);
		m_nodes[_i1] = _n1;
	}

	TrieBranchNode(byte _i1, MemTrieNode* _n1, byte _i2, MemTrieNode* _n2)
	{
		memset(m_nodes.data(), 0, sizeof(MemTrieNode*) * 16);
		m_nodes[_i1] = _n1;
		m_nodes[_i2] = _n2;
	}

	virtual ~TrieBranchNode()
	{
		for (auto i: m_nodes)
			delete i;
	}

#if ENABLE_DEBUG_PRINT
	virtual void debugPrintBody(std::string const& _indent) const
	{

		if (m_value.size())
			std::cerr << _indent << "@: " << m_value << std::endl;
		for (auto i = 0; i < 16; ++i)
			if (m_nodes[i])
			{
				std::cerr << _indent << std::hex << i << ": " << std::dec;
				m_nodes[i]->debugPrint(_indent + "  ");
			}
	}
#endif

	virtual std::string const& at(bytesConstRef _key) const override;
	virtual MemTrieNode* insert(bytesConstRef _key, std::string const& _value) override;
	virtual MemTrieNode* remove(bytesConstRef _key) override;
	virtual void makeRLP(RLPStream& _parentStream) const override;

private:
	/// @returns (byte)-1 when no active branches, 16 when multiple active and the index of the active branch otherwise.
	byte activeBranch() const;

	MemTrieNode* rejig();

	std::array<MemTrieNode*, 16> m_nodes;
	std::string m_value;
};

class TrieLeafNode: public TrieExtNode
{
public:
	TrieLeafNode(bytesConstRef _key, std::string const& _value): TrieExtNode(_key), m_value(_value) {}

#if ENABLE_DEBUG_PRINT
	virtual void debugPrintBody(std::string const& _indent) const
	{
		assert(m_value.size());
		std::cerr << _indent;
		if (m_ext.size())
			std::cerr << toHex(m_ext, 1) << ": ";
		else
			std::cerr << "@: ";
		std::cerr << m_value << std::endl;
	}
#endif

	virtual std::string const& at(bytesConstRef _key) const override { return contains(_key) ? m_value : c_nullString; }
	virtual MemTrieNode* insert(bytesConstRef _key, std::string const& _value) override;
	virtual MemTrieNode* remove(bytesConstRef _key) override;
	virtual void makeRLP(RLPStream& _parentStream) const override;

private:
	bool contains(bytesConstRef _key) const { return _key.size() == m_ext.size() && !memcmp(_key.data(), m_ext.data(), _key.size()); }

	std::string m_value;
};

class TrieInfixNode: public TrieExtNode
{
public:
	TrieInfixNode(bytesConstRef _key, MemTrieNode* _next): TrieExtNode(_key), m_next(_next) {}
	virtual ~TrieInfixNode() { delete m_next; }

#if ENABLE_DEBUG_PRINT
	virtual void debugPrintBody(std::string const& _indent) const
	{
		std::cerr << _indent << toHex(m_ext, 1) << ": ";
		m_next->debugPrint(_indent + "  ");
	}
#endif

	virtual std::string const& at(bytesConstRef _key) const override { assert(m_next); return contains(_key) ? m_next->at(_key.cropped(m_ext.size())) : c_nullString; }
	virtual MemTrieNode* insert(bytesConstRef _key, std::string const& _value) override;
	virtual MemTrieNode* remove(bytesConstRef _key) override;
	virtual void makeRLP(RLPStream& _parentStream) const override;

private:
	bool contains(bytesConstRef _key) const { return _key.size() >= m_ext.size() && !memcmp(_key.data(), m_ext.data(), m_ext.size()); }

	MemTrieNode* m_next;
};

void MemTrieNode::putRLP(RLPStream& _parentStream) const
{
	RLPStream s;
	makeRLP(s);
	if (s.out().size() < 32)
		_parentStream.APPEND_CHILD(s.out());
	else
		_parentStream << dev::sha3(s.out());
}

void TrieBranchNode::makeRLP(RLPStream& _intoStream) const
{
	_intoStream.appendList(17);
	for (auto i: m_nodes)
		if (i)
			i->putRLP(_intoStream);
		else
			_intoStream << "";
	_intoStream << m_value;
}

void TrieLeafNode::makeRLP(RLPStream& _intoStream) const
{
	_intoStream.appendList(2) << hexPrefixEncode(m_ext, true) << m_value;
}

void TrieInfixNode::makeRLP(RLPStream& _intoStream) const
{
	assert(m_next);
	_intoStream.appendList(2);
	_intoStream << hexPrefixEncode(m_ext, false);
	m_next->putRLP(_intoStream);
}

MemTrieNode* MemTrieNode::newBranch(bytesConstRef _k1, std::string const& _v1, bytesConstRef _k2, std::string const& _v2)
{
	unsigned prefix = commonPrefix(_k1, _k2);

	MemTrieNode* ret;
	if (_k1.size() == prefix)
		ret = new TrieBranchNode(_k2[prefix], new TrieLeafNode(_k2.cropped(prefix + 1), _v2), _v1);
	else if (_k2.size() == prefix)
		ret = new TrieBranchNode(_k1[prefix], new TrieLeafNode(_k1.cropped(prefix + 1), _v1), _v2);
	else // both continue after split
		ret = new TrieBranchNode(_k1[prefix], new TrieLeafNode(_k1.cropped(prefix + 1), _v1), _k2[prefix], new TrieLeafNode(_k2.cropped(prefix + 1), _v2));

	if (prefix)
		// have shared prefix - split.
		ret = new TrieInfixNode(_k1.cropped(0, prefix), ret);

	return ret;
}

std::string const& TrieBranchNode::at(bytesConstRef _key) const
{
	if (_key.empty())
		return m_value;
	else if (m_nodes[_key[0]] != nullptr)
		return m_nodes[_key[0]]->at(_key.cropped(1));
	return c_nullString;
}

MemTrieNode* TrieBranchNode::insert(bytesConstRef _key, std::string const& _value)
{
	assert(_value.size());
	mark();
	if (_key.empty())
		m_value = _value;
	else
		if (!m_nodes[_key[0]])
			m_nodes[_key[0]] = new TrieLeafNode(_key.cropped(1), _value);
		else
			m_nodes[_key[0]] = m_nodes[_key[0]]->insert(_key.cropped(1), _value);
	return this;
}

MemTrieNode* TrieBranchNode::remove(bytesConstRef _key)
{
	if (_key.empty())
		if (m_value.size())
		{
			m_value.clear();
			return rejig();
		}
		else {}
	else if (m_nodes[_key[0]] != nullptr)
	{
		m_nodes[_key[0]] = m_nodes[_key[0]]->remove(_key.cropped(1));
		return rejig();
	}
	return this;
}

MemTrieNode* TrieBranchNode::rejig()
{
	mark();
	byte n = activeBranch();

	if (n == (byte)-1 && m_value.size())
	{
		// switch to leaf
		auto r = new TrieLeafNode(bytesConstRef(), m_value);
		delete this;
		return r;
	}
	else if (n < 16 && m_value.empty())
	{
		// only branching to n...
		if (auto b = dynamic_cast<TrieBranchNode*>(m_nodes[n]))
		{
			// switch to infix
			m_nodes[n] = nullptr;
			delete this;
			return new TrieInfixNode(bytesConstRef(&n, 1), b);
		}
		else
		{
			auto x = dynamic_cast<TrieExtNode*>(m_nodes[n]);
			assert(x);
			// include in child
			pushFront(x->m_ext, n);
			m_nodes[n] = nullptr;
			delete this;
			return x;
		}
	}

	return this;
}

byte TrieBranchNode::activeBranch() const
{
	byte n = (byte)-1;
	for (int i = 0; i < 16; ++i)
		if (m_nodes[i] != nullptr)
		{
			if (n == (byte)-1)
				n = (byte)i;
			else
				return 16;
		}
	return n;
}

MemTrieNode* TrieInfixNode::insert(bytesConstRef _key, std::string const& _value)
{
	assert(_value.size());
	mark();
	if (contains(_key))
	{
		m_next = m_next->insert(_key.cropped(m_ext.size()), _value);
		return this;
	}
	else
	{
		unsigned prefix = commonPrefix(_key, m_ext);
		if (prefix)
		{
			// one infix becomes two infixes, then insert into the second
			// instead of pop_front()...
			trimFront(m_ext, prefix);

			return new TrieInfixNode(_key.cropped(0, prefix), insert(_key.cropped(prefix), _value));
		}
		else
		{
			// split here.
			auto f = m_ext[0];
			trimFront(m_ext, 1);
			MemTrieNode* n = m_ext.empty() ? m_next : this;
			if (n != this)
			{
				m_next = nullptr;
				delete this;
			}
			TrieBranchNode* ret = new TrieBranchNode(f, n);
			ret->insert(_key, _value);
			return ret;
		}
	}
}

MemTrieNode* TrieInfixNode::remove(bytesConstRef _key)
{
	if (contains(_key))
	{
		mark();
		m_next = m_next->remove(_key.cropped(m_ext.size()));
		if (auto p = dynamic_cast<TrieExtNode*>(m_next))
		{
			// merge with child...
			m_ext.reserve(m_ext.size() + p->m_ext.size());
			for (auto i: p->m_ext)
				m_ext.push_back(i);
			p->m_ext = m_ext;
			p->mark();
			m_next = nullptr;
			delete this;
			return p;
		}
		if (!m_next)
		{
			delete this;
			return nullptr;
		}
	}
	return this;
}

MemTrieNode* TrieLeafNode::insert(bytesConstRef _key, std::string const& _value)
{
	assert(_value.size());
	mark();
	if (contains(_key))
	{
		m_value = _value;
		return this;
	}
	else
	{
		// create new trie.
		auto n = MemTrieNode::newBranch(_key, _value, bytesConstRef(&m_ext), m_value);
		delete this;
		return n;
	}
}

MemTrieNode* TrieLeafNode::remove(bytesConstRef _key)
{
	if (contains(_key))
	{
		delete this;
		return nullptr;
	}
	return this;
}

MemTrie::~MemTrie()
{
	delete m_root;
}

h256 MemTrie::hash256() const
{
	return m_root ? m_root->hash256() : sha3(dev::rlp(bytesConstRef()));
}

bytes MemTrie::rlp() const
{
	return m_root ? m_root->rlp() : dev::rlp(bytesConstRef());
}

void MemTrie::debugPrint()
{
#if ENABLE_DEBUG_PRINT
	if (m_root)
		m_root->debugPrint();
#endif
}

std::string const& MemTrie::at(std::string const& _key) const
{
	if (!m_root)
		return c_nullString;
	auto h = asNibbles(_key);
	return m_root->at(bytesConstRef(&h));
}

void MemTrie::insert(std::string const& _key, std::string const& _value)
{
	if (_value.empty())
		remove(_key);
	auto h = asNibbles(_key);
	m_root = m_root ? m_root->insert(&h, _value) : new TrieLeafNode(bytesConstRef(&h), _value);
}

void MemTrie::remove(std::string const& _key)
{
	if (m_root)
	{
		auto h = asNibbles(_key);
		m_root = m_root->remove(&h);
	}
}

}

