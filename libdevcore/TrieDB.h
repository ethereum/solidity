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
/** @file TrieDB.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <memory>
#include "db.h"
#include "Common.h"
#include "Log.h"
#include "Exceptions.h"
#include "SHA3.h"
#include "MemoryDB.h"
#include "TrieCommon.h"

namespace dev
{

struct TrieDBChannel: public LogChannel  { static const char* name(); static const int verbosity = 17; };
#define tdebug clog(TrieDBChannel)

struct InvalidTrie: virtual dev::Exception {};
extern const h256 c_shaNull;
extern const h256 EmptyTrie;

enum class Verification {
	Skip,
	Normal
};

/**
 * @brief Merkle Patricia Tree "Trie": a modifed base-16 Radix tree.
 * This version uses a database backend.
 * Usage:
 * @code
 * GenericTrieDB<MyDB> t(&myDB);
 * assert(t.isNull());
 * t.init();
 * assert(t.isEmpty());
 * t.insert(x, y);
 * assert(t.at(x) == y.toString());
 * t.remove(x);
 * assert(t.isEmpty());
 * @endcode
 */
template <class _DB>
class GenericTrieDB
{
public:
	using DB = _DB;

	explicit GenericTrieDB(DB* _db = nullptr): m_db(_db) {}
	GenericTrieDB(DB* _db, h256 const& _root, Verification _v = Verification::Normal) { open(_db, _root, _v); }
	~GenericTrieDB() {}

	void open(DB* _db) { m_db = _db; }
	void open(DB* _db, h256 const& _root, Verification _v = Verification::Normal) { m_db = _db; setRoot(_root, _v); }

	void init() { setRoot(forceInsertNode(&RLPNull)); assert(node(m_root).size()); }

	void setRoot(h256 const& _root, Verification _v = Verification::Normal)
	{
		m_root = _root;
		if (_v == Verification::Normal)
		{
			if (m_root == c_shaNull && !m_db->exists(m_root))
				init();
		}
		/*std::cout << "Setting root to " << _root << " (patched to " << m_root << ")" << std::endl;*/
#if ETH_DEBUG
		if (_v == Verification::Normal)
#endif
			if (!node(m_root).size())
				BOOST_THROW_EXCEPTION(RootNotFound());
	}

	/// True if the trie is uninitialised (i.e. that the DB doesn't contain the root node).
	bool isNull() const { return !node(m_root).size(); }
	/// True if the trie is initialised but empty (i.e. that the DB contains the root node which is empty).
	bool isEmpty() const { return m_root == c_shaNull && node(m_root).size(); }

	h256 const& root() const { if (node(m_root).empty()) BOOST_THROW_EXCEPTION(BadRoot(m_root)); /*std::cout << "Returning root as " << ret << " (really " << m_root << ")" << std::endl;*/ return m_root; }	// patch the root in the case of the empty trie. TODO: handle this properly.

	std::string at(bytes const& _key) const { return at(&_key); }
	std::string at(bytesConstRef _key) const;
	void insert(bytes const& _key, bytes const& _value) { insert(&_key, &_value); }
	void insert(bytesConstRef _key, bytes const& _value) { insert(_key, &_value); }
	void insert(bytes const& _key, bytesConstRef _value) { insert(&_key, _value); }
	void insert(bytesConstRef _key, bytesConstRef _value);
	void remove(bytes const& _key) { remove(&_key); }
	void remove(bytesConstRef _key);
	bool contains(bytes const& _key) { return contains(&_key); }
	bool contains(bytesConstRef _key) { return !at(_key).empty(); }

	class iterator
	{
	public:
		using value_type = std::pair<bytesConstRef, bytesConstRef>;

		iterator() {}
		explicit iterator(GenericTrieDB const* _db);
		iterator(GenericTrieDB const* _db, bytesConstRef _key);

		iterator& operator++() { next(); return *this; }

		value_type operator*() const { return at(); }
		value_type operator->() const { return at(); }

		bool operator==(iterator const& _c) const { return _c.m_trail == m_trail; }
		bool operator!=(iterator const& _c) const { return _c.m_trail != m_trail; }

		value_type at() const;

	private:
		void next();
		void next(NibbleSlice _key);

		struct Node
		{
			std::string rlp;
			std::string key;		// as hexPrefixEncoding.
			byte child;				// 255 -> entering, 16 -> actually at the node, 17 -> exiting, 0-15 -> actual children.

			// 255 -> 16 -> 0 -> 1 -> ... -> 15 -> 17

			void setChild(unsigned _i) { child = _i; }
			void setFirstChild() { child = 16; }
			void incrementChild() { child = child == 16 ? 0 : child == 15 ? 17 : (child + 1); }

			bool operator==(Node const& _c) const { return rlp == _c.rlp && key == _c.key && child == _c.child; }
			bool operator!=(Node const& _c) const { return !operator==(_c); }
		};

	protected:
		std::vector<Node> m_trail;
		GenericTrieDB<DB> const* m_that;
	};

	iterator begin() const { return iterator(this); }
	iterator end() const { return iterator(); }

	iterator lower_bound(bytesConstRef _key) const { return iterator(this, _key); }

	/// Used for debugging, scans the whole trie.
	void descendKey(h256 const& _k, h256Hash& _keyMask, bool _wasExt, std::ostream* _out, int _indent = 0) const
	{
		_keyMask.erase(_k);
		if (_k == m_root && _k == c_shaNull)	// root allowed to be empty
			return;
		descendList(RLP(node(_k)), _keyMask, _wasExt, _out, _indent);	// if not, it must be a list
	}

	/// Used for debugging, scans the whole trie.
	void descendEntry(RLP const& _r, h256Hash& _keyMask, bool _wasExt, std::ostream* _out, int _indent) const
	{
		if (_r.isData() && _r.size() == 32)
			descendKey(_r.toHash<h256>(), _keyMask, _wasExt, _out, _indent);
		else if (_r.isList())
			descendList(_r, _keyMask, _wasExt, _out, _indent);
		else
			BOOST_THROW_EXCEPTION(InvalidTrie());
	}

	/// Used for debugging, scans the whole trie.
	void descendList(RLP const& _r, h256Hash& _keyMask, bool _wasExt, std::ostream* _out, int _indent) const
	{
		if (_r.isList() && _r.itemCount() == 2 && (!_wasExt || _out))
		{
			if (_out)
				(*_out) << std::string(_indent * 2, ' ') << (_wasExt ? "!2 " : "2  ") << sha3(_r.data()) << ": " << _r << "\n";
			if (!isLeaf(_r))						// don't go down leaves
				descendEntry(_r[1], _keyMask, true, _out, _indent + 1);
		}
		else if (_r.isList() && _r.itemCount() == 17)
		{
			if (_out)
				(*_out) << std::string(_indent * 2, ' ') << "17 " << sha3(_r.data()) << ": " << _r << "\n";
			for (unsigned i = 0; i < 16; ++i)
				if (!_r[i].isEmpty())				// 16 branches are allowed to be empty
					descendEntry(_r[i], _keyMask, false, _out, _indent + 1);
		}
		else
			BOOST_THROW_EXCEPTION(InvalidTrie());
	}

	/// Used for debugging, scans the whole trie.
	h256Hash leftOvers(std::ostream* _out = nullptr) const
	{
		h256Hash k = m_db->keys();
		descendKey(m_root, k, false, _out);
		return k;
	}

	/// Used for debugging, scans the whole trie.
	void debugStructure(std::ostream& _out) const
	{
		leftOvers(&_out);
	}

	/// Used for debugging, scans the whole trie.
	/// @param _requireNoLeftOvers if true, requires that all keys are reachable.
	bool check(bool _requireNoLeftOvers) const
	{
		try
		{
			return leftOvers().empty() || !_requireNoLeftOvers;
		}
		catch (...)
		{
			cwarn << boost::current_exception_diagnostic_information();
			return false;
		}
	}

	/// Get the underlying database.
	/// @warning This can be used to bypass the trie code. Don't use these unless you *really*
	/// know what you're doing.
	DB const* db() const { return m_db; }
	DB* db() { return m_db; }

private:
	RLPStream& streamNode(RLPStream& _s, bytes const& _b);

	std::string atAux(RLP const& _here, NibbleSlice _key) const;

	void mergeAtAux(RLPStream& _out, RLP const& _replace, NibbleSlice _key, bytesConstRef _value);
	bytes mergeAt(RLP const& _replace, NibbleSlice _k, bytesConstRef _v, bool _inLine = false);
	bytes mergeAt(RLP const& _replace, h256 const& _replaceHash, NibbleSlice _k, bytesConstRef _v, bool _inLine = false);

	bool deleteAtAux(RLPStream& _out, RLP const& _replace, NibbleSlice _key);
	bytes deleteAt(RLP const& _replace, NibbleSlice _k);

	// in: null (DEL)  -- OR --  [_k, V] (DEL)
	// out: [_k, _s]
	// -- OR --
	// in: [V0, ..., V15, S16] (DEL)  AND  _k == {}
	// out: [V0, ..., V15, _s]
	bytes place(RLP const& _orig, NibbleSlice _k, bytesConstRef _s);

	// in: [K, S] (DEL)
	// out: null
	// -- OR --
	// in: [V0, ..., V15, S] (DEL)
	// out: [V0, ..., V15, null]
	bytes remove(RLP const& _orig);

	// in: [K1 & K2, V] (DEL) : nibbles(K1) == _s, 0 < _s <= nibbles(K1 & K2)
	// out: [K1, H] ; [K2, V] => H (INS)  (being  [K1, [K2, V]]  if necessary)
	bytes cleve(RLP const& _orig, unsigned _s);

	// in: [K1, H] (DEL) ; H <= [K2, V] (DEL)  (being  [K1, [K2, V]] (DEL)  if necessary)
	// out: [K1 & K2, V]
	bytes graft(RLP const& _orig);

	// in: [V0, ... V15, S] (DEL)
	// out1: [k{i}, Vi]    where i < 16
	// out2: [k{}, S]      where i == 16
	bytes merge(RLP const& _orig, byte _i);

	// in: [k{}, S] (DEL)
	// out: [null ** 16, S]
	// -- OR --
	// in: [k{i}, N] (DEL)
	// out: [null ** i, N, null ** (16 - i)]
	// -- OR --
	// in: [k{i}K, V] (DEL)
	// out: [null ** i, H, null ** (16 - i)] ; [K, V] => H (INS)  (being [null ** i, [K, V], null ** (16 - i)]  if necessary)
	bytes branch(RLP const& _orig);

	bool isTwoItemNode(RLP const& _n) const;
	std::string deref(RLP const& _n) const;

	std::string node(h256 const& _h) const { return m_db->lookup(_h); }

	// These are low-level node insertion functions that just go straight through into the DB.
	h256 forceInsertNode(bytesConstRef _v) { auto h = sha3(_v); forceInsertNode(h, _v); return h; }
	void forceInsertNode(h256 const& _h, bytesConstRef _v) { m_db->insert(_h, _v); }
	void forceKillNode(h256 const& _h) { m_db->kill(_h); }

	// This are semantically-aware node insertion functions that only kills when the node's
	// data is < 32 bytes. It can safely be used when pruning the trie but won't work correctly
	// for the special case of the root (which is always looked up via a hash). In that case,
	// use forceKillNode().
	void killNode(RLP const& _d) { if (_d.data().size() >= 32) forceKillNode(sha3(_d.data())); }
	void killNode(RLP const& _d, h256 const& _h) { if (_d.data().size() >= 32) forceKillNode(_h); }

	h256 m_root;
	DB* m_db = nullptr;
};

template <class DB>
std::ostream& operator<<(std::ostream& _out, GenericTrieDB<DB> const& _db)
{
	for (auto const& i: _db)
		_out << escaped(i.first.toString(), false) << ": " << escaped(i.second.toString(), false) << std::endl;
	return _out;
}

/**
 * Different view on a GenericTrieDB that can use different key types.
 */
template <class Generic, class _KeyType>
class SpecificTrieDB: public Generic
{
public:
	using DB = typename Generic::DB;
	using KeyType = _KeyType;

	SpecificTrieDB(DB* _db = nullptr): Generic(_db) {}
	SpecificTrieDB(DB* _db, h256 _root, Verification _v = Verification::Normal): Generic(_db, _root, _v) {}

	std::string operator[](KeyType _k) const { return at(_k); }

	bool contains(KeyType _k) const { return Generic::contains(bytesConstRef((byte const*)&_k, sizeof(KeyType))); }
	std::string at(KeyType _k) const { return Generic::at(bytesConstRef((byte const*)&_k, sizeof(KeyType))); }
	void insert(KeyType _k, bytesConstRef _value) { Generic::insert(bytesConstRef((byte const*)&_k, sizeof(KeyType)), _value); }
	void insert(KeyType _k, bytes const& _value) { insert(_k, bytesConstRef(&_value)); }
	void remove(KeyType _k) { Generic::remove(bytesConstRef((byte const*)&_k, sizeof(KeyType))); }

	class iterator: public Generic::iterator
	{
	public:
		using Super = typename Generic::iterator;
		using value_type = std::pair<KeyType, bytesConstRef>;

		iterator() {}
		iterator(Generic const* _db): Super(_db) {}
		iterator(Generic const* _db, bytesConstRef _k): Super(_db, _k) {}

		value_type operator*() const { return at(); }
		value_type operator->() const { return at(); }

		value_type at() const;
	};

	iterator begin() const { return this; }
	iterator end() const { return iterator(); }
	iterator lower_bound(KeyType _k) const { return iterator(this, bytesConstRef((byte const*)&_k, sizeof(KeyType))); }
};

template <class Generic, class KeyType>
std::ostream& operator<<(std::ostream& _out, SpecificTrieDB<Generic, KeyType> const& _db)
{
	for (auto const& i: _db)
		_out << i.first << ": " << escaped(i.second.toString(), false) << std::endl;
	return _out;
}

template <class _DB>
class HashedGenericTrieDB: private SpecificTrieDB<GenericTrieDB<_DB>, h256>
{
	using Super = SpecificTrieDB<GenericTrieDB<_DB>, h256>;

public:
	using DB = _DB;

	HashedGenericTrieDB(DB* _db = nullptr): Super(_db) {}
	HashedGenericTrieDB(DB* _db, h256 _root, Verification _v = Verification::Normal): Super(_db, _root, _v) {}

	using Super::open;
	using Super::init;
	using Super::setRoot;

	/// True if the trie is uninitialised (i.e. that the DB doesn't contain the root node).
	using Super::isNull;
	/// True if the trie is initialised but empty (i.e. that the DB contains the root node which is empty).
	using Super::isEmpty;

	using Super::root;
	using Super::db;

	using Super::leftOvers;
	using Super::check;
	using Super::debugStructure;

	std::string at(bytesConstRef _key) const { return Super::at(sha3(_key)); }
	bool contains(bytesConstRef _key) { return Super::contains(sha3(_key)); }
	void insert(bytesConstRef _key, bytesConstRef _value) { Super::insert(sha3(_key), _value); }
	void remove(bytesConstRef _key) { Super::remove(sha3(_key)); }

	// empty from the PoV of the iterator interface; still need a basic iterator impl though.
	class iterator
	{
	public:
		using value_type = std::pair<bytesConstRef, bytesConstRef>;

		iterator() {}
		iterator(HashedGenericTrieDB const*) {}
		iterator(HashedGenericTrieDB const*, bytesConstRef) {}

		iterator& operator++() { return *this; }
		value_type operator*() const { return value_type(); }
		value_type operator->() const { return value_type(); }

		bool operator==(iterator const&) const { return true; }
		bool operator!=(iterator const&) const { return false; }

		value_type at() const { return value_type(); }
	};
	iterator begin() const { return iterator(); }
	iterator end() const { return iterator(); }
	iterator lower_bound(bytesConstRef) const { return iterator(); }
};

// Hashed & Hash-key mapping
template <class _DB>
class FatGenericTrieDB: private SpecificTrieDB<GenericTrieDB<_DB>, h256>
{
	using Super = SpecificTrieDB<GenericTrieDB<_DB>, h256>;

public:
	using DB = _DB;
	FatGenericTrieDB(DB* _db = nullptr): Super(_db) {}
	FatGenericTrieDB(DB* _db, h256 _root, Verification _v = Verification::Normal): Super(_db, _root, _v) {}

	using Super::init;
	using Super::isNull;
	using Super::isEmpty;
	using Super::root;
	using Super::leftOvers;
	using Super::check;
	using Super::open;
	using Super::setRoot;
	using Super::db;
	using Super::debugStructure;

	std::string at(bytesConstRef _key) const { return Super::at(sha3(_key)); }
	bool contains(bytesConstRef _key) { return Super::contains(sha3(_key)); }
	void insert(bytesConstRef _key, bytesConstRef _value)
	{
		h256 hash = sha3(_key);
		Super::insert(hash, _value);
		Super::db()->insertAux(hash, _key);
	}

	void remove(bytesConstRef _key) { Super::remove(sha3(_key)); }

	//friend class iterator;

	class iterator : public GenericTrieDB<_DB>::iterator
	{
	public:
		using Super = typename GenericTrieDB<_DB>::iterator;

		iterator() { }
		iterator(FatGenericTrieDB const* _trie): Super(_trie) { }

		typename Super::value_type at() const
		{
			auto hashed = Super::at();
			m_key = static_cast<FatGenericTrieDB const*>(Super::m_that)->db()->lookupAux(h256(hashed.first));
			return std::make_pair(&m_key, std::move(hashed.second));
		}

	private:
		mutable bytes m_key;
	};
	iterator begin() const { return iterator(); }
	iterator end() const { return iterator(); }
};

template <class KeyType, class DB> using TrieDB = SpecificTrieDB<GenericTrieDB<DB>, KeyType>;

}

// Template implementations...
namespace dev
{

template <class DB> GenericTrieDB<DB>::iterator::iterator(GenericTrieDB const* _db)
{
	m_that = _db;
	m_trail.push_back({_db->node(_db->m_root), std::string(1, '\0'), 255});	// one null byte is the HPE for the empty key.
	next();
}

template <class DB> GenericTrieDB<DB>::iterator::iterator(GenericTrieDB const* _db, bytesConstRef _fullKey)
{
	m_that = _db;
	m_trail.push_back({_db->node(_db->m_root), std::string(1, '\0'), 255});	// one null byte is the HPE for the empty key.
	next(_fullKey);
}

template <class DB> typename GenericTrieDB<DB>::iterator::value_type GenericTrieDB<DB>::iterator::at() const
{
	assert(m_trail.size());
	Node const& b = m_trail.back();
	assert(b.key.size());
	assert(!(b.key[0] & 0x10));	// should be an integer number of bytes (i.e. not an odd number of nibbles).

	RLP rlp(b.rlp);
	return std::make_pair(bytesConstRef(b.key).cropped(1), rlp[rlp.itemCount() == 2 ? 1 : 16].payload());
}

template <class DB> void GenericTrieDB<DB>::iterator::next(NibbleSlice _key)
{
	NibbleSlice k = _key;
	while (true)
	{
		if (m_trail.empty())
		{
			m_that = nullptr;
			return;
		}

		Node const& b = m_trail.back();
		RLP rlp(b.rlp);

		if (m_trail.back().child == 255)
		{
			// Entering. Look for first...
			if (rlp.isEmpty())
			{
				// Kill our search as soon as we hit an empty node.
				k.clear();
				m_trail.pop_back();
				continue;
			}
			if (!rlp.isList() || (rlp.itemCount() != 2 && rlp.itemCount() != 17))
			{
#if ETH_PARANOIA
				cwarn << "BIG FAT ERROR. STATE TRIE CORRUPTED!!!!!";
				cwarn << b.rlp.size() << toHex(b.rlp);
				cwarn << rlp;
				auto c = rlp.itemCount();
				cwarn << c;
				BOOST_THROW_EXCEPTION(InvalidTrie());
#else
				m_that = nullptr;
				return;
#endif
			}
			if (rlp.itemCount() == 2)
			{
				// Just turn it into a valid Branch
				auto keyOfRLP = keyOf(rlp);

				// TODO: do something different depending on how keyOfRLP compares to k.mid(0, std::min(k.size(), keyOfRLP.size()));
				// if == all is good - continue descent.
				// if > discard key and continue descent.
				// if < discard key and skip node.

				if (!k.contains(keyOfRLP))
				{
					if (!k.isEarlierThan(keyOfRLP))
					{
						k.clear();
						m_trail.pop_back();
						continue;
					}
					k.clear();
				}

				k = k.mid(std::min(k.size(), keyOfRLP.size()));
				m_trail.back().key = hexPrefixEncode(keyOf(m_trail.back().key), keyOfRLP, false);
				if (isLeaf(rlp))
				{
					// leaf - exit now.
					if (k.empty())
					{
						m_trail.back().child = 0;
						return;
					}
					// Still data in key we're supposed to be looking for when we're at a leaf. Go for next one.
					k.clear();
					m_trail.pop_back();
					continue;
				}

				// enter child.
				m_trail.back().rlp = m_that->deref(rlp[1]);
				// no need to set .child as 255 - it's already done.
				continue;
			}
			else
			{
				// Already a branch - look for first valid.
				if (k.size())
				{
					m_trail.back().setChild(k[0]);
					k = k.mid(1);
				}
				else
					m_trail.back().setChild(16);
				// run through to...
			}
		}
		else
		{
			// Continuing/exiting. Look for next...
			if (!(rlp.isList() && rlp.itemCount() == 17))
			{
				k.clear();
				m_trail.pop_back();
				continue;
			}
			// else run through to...
			m_trail.back().incrementChild();
		}

		// ...here. should only get here if we're a list.
		assert(rlp.isList() && rlp.itemCount() == 17);
		for (;; m_trail.back().incrementChild())
			if (m_trail.back().child == 17)
			{
				// finished here.
				k.clear();
				m_trail.pop_back();
				break;
			}
			else if (!rlp[m_trail.back().child].isEmpty())
			{
				if (m_trail.back().child == 16)
					return;	// have a value at this node - exit now.
				else
				{
					// lead-on to another node - enter child.
					// fixed so that Node passed into push_back is constructed *before* m_trail is potentially resized (which invalidates back and rlp)
					Node const& back = m_trail.back();
					m_trail.push_back(Node{
						m_that->deref(rlp[back.child]),
						 hexPrefixEncode(keyOf(back.key), NibbleSlice(bytesConstRef(&back.child, 1), 1), false),
						 255
						});
					break;
				}
			}
		else
			k.clear();
	}
}

template <class DB> void GenericTrieDB<DB>::iterator::next()
{
	while (true)
	{
		if (m_trail.empty())
		{
			m_that = nullptr;
			return;
		}

		Node const& b = m_trail.back();
		RLP rlp(b.rlp);

		if (m_trail.back().child == 255)
		{
			// Entering. Look for first...
			if (rlp.isEmpty())
			{
				m_trail.pop_back();
				continue;
			}
			if (!(rlp.isList() && (rlp.itemCount() == 2 || rlp.itemCount() == 17)))
			{
#if ETH_PARANOIA
				cwarn << "BIG FAT ERROR. STATE TRIE CORRUPTED!!!!!";
				cwarn << b.rlp.size() << toHex(b.rlp);
				cwarn << rlp;
				auto c = rlp.itemCount();
				cwarn << c;
				BOOST_THROW_EXCEPTION(InvalidTrie());
#else
				m_that = nullptr;
				return;
#endif
			}
			if (rlp.itemCount() == 2)
			{
				// Just turn it into a valid Branch
				m_trail.back().key = hexPrefixEncode(keyOf(m_trail.back().key), keyOf(rlp), false);
				if (isLeaf(rlp))
				{
					// leaf - exit now.
					m_trail.back().child = 0;
					return;
				}

				// enter child.
				m_trail.back().rlp = m_that->deref(rlp[1]);
				// no need to set .child as 255 - it's already done.
				continue;
			}
			else
			{
				// Already a branch - look for first valid.
				m_trail.back().setFirstChild();
				// run through to...
			}
		}
		else
		{
			// Continuing/exiting. Look for next...
			if (!(rlp.isList() && rlp.itemCount() == 17))
			{
				m_trail.pop_back();
				continue;
			}
			// else run through to...
			m_trail.back().incrementChild();
		}

		// ...here. should only get here if we're a list.
		assert(rlp.isList() && rlp.itemCount() == 17);
		for (;; m_trail.back().incrementChild())
			if (m_trail.back().child == 17)
			{
				// finished here.
				m_trail.pop_back();
				break;
			}
			else if (!rlp[m_trail.back().child].isEmpty())
			{
				if (m_trail.back().child == 16)
					return;	// have a value at this node - exit now.
				else
				{
					// lead-on to another node - enter child.
					// fixed so that Node passed into push_back is constructed *before* m_trail is potentially resized (which invalidates back and rlp)
					Node const& back = m_trail.back();
					m_trail.push_back(Node{
						m_that->deref(rlp[back.child]),
						 hexPrefixEncode(keyOf(back.key), NibbleSlice(bytesConstRef(&back.child, 1), 1), false),
						 255
						});
					break;
				}
			}
	}
}

template <class KeyType, class DB> typename SpecificTrieDB<KeyType, DB>::iterator::value_type SpecificTrieDB<KeyType, DB>::iterator::at() const
{
	auto p = Super::at();
	value_type ret;
	assert(p.first.size() == sizeof(KeyType));
	memcpy(&ret.first, p.first.data(), sizeof(KeyType));
	ret.second = p.second;
	return ret;
}

template <class DB> void GenericTrieDB<DB>::insert(bytesConstRef _key, bytesConstRef _value)
{
#if ETH_PARANOIA
	tdebug << "Insert" << toHex(_key.cropped(0, 4)) << "=>" << toHex(_value);
#endif

	std::string rootValue = node(m_root);
	assert(rootValue.size());
	bytes b = mergeAt(RLP(rootValue), m_root, NibbleSlice(_key), _value);

	// mergeAt won't attempt to delete the node if it's less than 32 bytes
	// However, we know it's the root node and thus always hashed.
	// So, if it's less than 32 (and thus should have been deleted but wasn't) then we delete it here.
	if (rootValue.size() < 32)
		forceKillNode(m_root);
	m_root = forceInsertNode(&b);
}

template <class DB> std::string GenericTrieDB<DB>::at(bytesConstRef _key) const
{
	return atAux(RLP(node(m_root)), _key);
}

template <class DB> std::string GenericTrieDB<DB>::atAux(RLP const& _here, NibbleSlice _key) const
{
	if (_here.isEmpty() || _here.isNull())
		// not found.
		return std::string();
	unsigned itemCount = _here.itemCount();
	assert(_here.isList() && (itemCount == 2 || itemCount == 17));
	if (itemCount == 2)
	{
		auto k = keyOf(_here);
		if (_key == k && isLeaf(_here))
			// reached leaf and it's us
			return _here[1].toString();
		else if (_key.contains(k) && !isLeaf(_here))
			// not yet at leaf and it might yet be us. onwards...
			return atAux(_here[1].isList() ? _here[1] : RLP(node(_here[1].toHash<h256>())), _key.mid(k.size()));
		else
			// not us.
			return std::string();
	}
	else
	{
		if (_key.size() == 0)
			return _here[16].toString();
		auto n = _here[_key[0]];
		if (n.isEmpty())
			return std::string();
		else
			return atAux(n.isList() ? n : RLP(node(n.toHash<h256>())), _key.mid(1));
	}
}

template <class DB> bytes GenericTrieDB<DB>::mergeAt(RLP const& _orig, NibbleSlice _k, bytesConstRef _v, bool _inLine)
{
	return mergeAt(_orig, sha3(_orig.data()), _k, _v, _inLine);
}

template <class DB> bytes GenericTrieDB<DB>::mergeAt(RLP const& _orig, h256 const& _origHash, NibbleSlice _k, bytesConstRef _v, bool _inLine)
{
#if ETH_PARANOIA
	tdebug << "mergeAt " << _orig << _k << sha3(_orig.data());
#endif

	// The caller will make sure that the bytes are inserted properly.
	// - This might mean inserting an entry into m_over
	// We will take care to ensure that (our reference to) _orig is killed.

	// Empty - just insert here
	if (_orig.isEmpty())
		return place(_orig, _k, _v);

	unsigned itemCount = _orig.itemCount();
	assert(_orig.isList() && (itemCount == 2 || itemCount == 17));
	if (itemCount == 2)
	{
		// pair...
		NibbleSlice k = keyOf(_orig);

		// exactly our node - place value in directly.
		if (k == _k && isLeaf(_orig))
			return place(_orig, _k, _v);

		// partial key is our key - move down.
		if (_k.contains(k) && !isLeaf(_orig))
		{
			if (!_inLine)
				killNode(_orig, _origHash);
			RLPStream s(2);
			s.append(_orig[0]);
			mergeAtAux(s, _orig[1], _k.mid(k.size()), _v);
			return s.out();
		}

		auto sh = _k.shared(k);
//		std::cout << _k << " sh " << k << " = " << sh << std::endl;
		if (sh)
		{
			// shared stuff - cleve at disagreement.
			auto cleved = cleve(_orig, sh);
			return mergeAt(RLP(cleved), _k, _v, true);
		}
		else
		{
			// nothing shared - branch
			auto branched = branch(_orig);
			return mergeAt(RLP(branched), _k, _v, true);
		}
	}
	else
	{
		// branch...

		// exactly our node - place value.
		if (_k.size() == 0)
			return place(_orig, _k, _v);

		// Kill the node.
		if (!_inLine)
			killNode(_orig, _origHash);

		// not exactly our node - delve to next level at the correct index.
		byte n = _k[0];
		RLPStream r(17);
		for (byte i = 0; i < 17; ++i)
			if (i == n)
				mergeAtAux(r, _orig[i], _k.mid(1), _v);
			else
				r.append(_orig[i]);
		return r.out();
	}

}

template <class DB> void GenericTrieDB<DB>::mergeAtAux(RLPStream& _out, RLP const& _orig, NibbleSlice _k, bytesConstRef _v)
{
#if ETH_PARANOIA || !ETH_TRUE
	tdebug << "mergeAtAux " << _orig << _k << sha3(_orig.data()) << ((_orig.isData() && _orig.size() <= 32) ? _orig.toHash<h256>().abridged() : std::string());
#endif

	RLP r = _orig;
	std::string s;
	// _orig is always a segment of a node's RLP - removing it alone is pointless. However, if may be a hash, in which case we deref and we know it is removable.
	bool isRemovable = false;
	if (!r.isList() && !r.isEmpty())
	{
		s = node(_orig.toHash<h256>());
		r = RLP(s);
		assert(!r.isNull());
		isRemovable = true;
	}
	bytes b = mergeAt(r, _k, _v, !isRemovable);
	streamNode(_out, b);
}

template <class DB> void GenericTrieDB<DB>::remove(bytesConstRef _key)
{
#if ETH_PARANOIA
	tdebug << "Remove" << toHex(_key.cropped(0, 4).toBytes());
#endif

	std::string rv = node(m_root);
	bytes b = deleteAt(RLP(rv), NibbleSlice(_key));
	if (b.size())
	{
		if (rv.size() < 32)
			forceKillNode(m_root);
		m_root = forceInsertNode(&b);
	}
}

template <class DB> bool GenericTrieDB<DB>::isTwoItemNode(RLP const& _n) const
{
	return (_n.isData() && RLP(node(_n.toHash<h256>())).itemCount() == 2)
			|| (_n.isList() && _n.itemCount() == 2);
}

template <class DB> std::string GenericTrieDB<DB>::deref(RLP const& _n) const
{
	return _n.isList() ? _n.data().toString() : node(_n.toHash<h256>());
}

template <class DB> bytes GenericTrieDB<DB>::deleteAt(RLP const& _orig, NibbleSlice _k)
{
#if ETH_PARANOIA
	tdebug << "deleteAt " << _orig << _k << sha3(_orig.data());
#endif

	// The caller will make sure that the bytes are inserted properly.
	// - This might mean inserting an entry into m_over
	// We will take care to ensure that (our reference to) _orig is killed.

	// Empty - not found - no change.
	if (_orig.isEmpty())
		return bytes();

	assert(_orig.isList() && (_orig.itemCount() == 2 || _orig.itemCount() == 17));
	if (_orig.itemCount() == 2)
	{
		// pair...
		NibbleSlice k = keyOf(_orig);

		// exactly our node - return null.
		if (k == _k && isLeaf(_orig))
		{
			killNode(_orig);
			return RLPNull;
		}

		// partial key is our key - move down.
		if (_k.contains(k))
		{
			RLPStream s;
			s.appendList(2) << _orig[0];
			if (!deleteAtAux(s, _orig[1], _k.mid(k.size())))
				return bytes();
			killNode(_orig);
			RLP r(s.out());
			if (isTwoItemNode(r[1]))
				return graft(r);
			return s.out();
		}
		else
			// not found - no change.
			return bytes();
	}
	else
	{
		// branch...

		// exactly our node - remove and rejig.
		if (_k.size() == 0 && !_orig[16].isEmpty())
		{
			// Kill the node.
			killNode(_orig);

			byte used = uniqueInUse(_orig, 16);
			if (used != 255)
				if (isTwoItemNode(_orig[used]))
				{
					auto merged = merge(_orig, used);
					return graft(RLP(merged));
				}
				else
					return merge(_orig, used);
			else
			{
				RLPStream r(17);
				for (byte i = 0; i < 16; ++i)
					r << _orig[i];
				r << "";
				return r.out();
			}
		}
		else
		{
			// not exactly our node - delve to next level at the correct index.
			RLPStream r(17);
			byte n = _k[0];
			for (byte i = 0; i < 17; ++i)
				if (i == n)
					if (!deleteAtAux(r, _orig[i], _k.mid(1)))	// bomb out if the key didn't turn up.
						return bytes();
					else {}
				else
					r << _orig[i];

			// Kill the node.
			killNode(_orig);

			// check if we ended up leaving the node invalid.
			RLP rlp(r.out());
			byte used = uniqueInUse(rlp, 255);
			if (used == 255)	// no - all ok.
				return r.out();

			// yes; merge
			if (isTwoItemNode(rlp[used]))
			{
				auto merged = merge(rlp, used);
				return graft(RLP(merged));
			}
			else
				return merge(rlp, used);
		}
	}

}

template <class DB> bool GenericTrieDB<DB>::deleteAtAux(RLPStream& _out, RLP const& _orig, NibbleSlice _k)
{
#if ETH_PARANOIA || !ETH_TRUE
	tdebug << "deleteAtAux " << _orig << _k << sha3(_orig.data()) << ((_orig.isData() && _orig.size() <= 32) ? _orig.toHash<h256>().abridged() : std::string());
#endif

	bytes b = _orig.isEmpty() ? bytes() : deleteAt(_orig.isList() ? _orig : RLP(node(_orig.toHash<h256>())), _k);

	if (!b.size())	// not found - no change.
		return false;

/*	if (_orig.isList())
		killNode(_orig);
	else
		killNode(_orig.toHash<h256>());*/

	streamNode(_out, b);
	return true;
}

template <class DB> bytes GenericTrieDB<DB>::place(RLP const& _orig, NibbleSlice _k, bytesConstRef _s)
{
#if ETH_PARANOIA
	tdebug << "place " << _orig << _k;
#endif

	killNode(_orig);
	if (_orig.isEmpty())
		return rlpList(hexPrefixEncode(_k, true), _s);

	assert(_orig.isList() && (_orig.itemCount() == 2 || _orig.itemCount() == 17));
	if (_orig.itemCount() == 2)
		return rlpList(_orig[0], _s);

	auto s = RLPStream(17);
	for (unsigned i = 0; i < 16; ++i)
		s << _orig[i];
	s << _s;
	return s.out();
}

// in1: [K, S] (DEL)
// out1: null
// in2: [V0, ..., V15, S] (DEL)
// out2: [V0, ..., V15, null] iff exists i: !!Vi  -- OR --  null otherwise
template <class DB> bytes GenericTrieDB<DB>::remove(RLP const& _orig)
{
#if ETH_PARANOIA
	tdebug << "kill " << _orig;
#endif

	killNode(_orig);

	assert(_orig.isList() && (_orig.itemCount() == 2 || _orig.itemCount() == 17));
	if (_orig.itemCount() == 2)
		return RLPNull;
	RLPStream r(17);
	for (unsigned i = 0; i < 16; ++i)
		r << _orig[i];
	r << "";
	return r.out();
}

template <class DB> RLPStream& GenericTrieDB<DB>::streamNode(RLPStream& _s, bytes const& _b)
{
	if (_b.size() < 32)
		_s.appendRaw(_b);
	else
		_s.append(forceInsertNode(&_b));
	return _s;
}

template <class DB> bytes GenericTrieDB<DB>::cleve(RLP const& _orig, unsigned _s)
{
#if ETH_PARANOIA
	tdebug << "cleve " << _orig << _s;
#endif

	killNode(_orig);
	assert(_orig.isList() && _orig.itemCount() == 2);
	auto k = keyOf(_orig);
	assert(_s && _s <= k.size());

	RLPStream bottom(2);
	bottom << hexPrefixEncode(k, isLeaf(_orig), /*ugh*/(int)_s) << _orig[1];

	RLPStream top(2);
	top << hexPrefixEncode(k, false, 0, /*ugh*/(int)_s);
	streamNode(top, bottom.out());

	return top.out();
}

template <class DB> bytes GenericTrieDB<DB>::graft(RLP const& _orig)
{
#if ETH_PARANOIA
	tdebug << "graft " << _orig;
#endif

	assert(_orig.isList() && _orig.itemCount() == 2);
	std::string s;
	RLP n;
	if (_orig[1].isList())
		n = _orig[1];
	else
	{
		// remove second item from the trie after derefrencing it into s & n.
		auto lh = _orig[1].toHash<h256>();
		s = node(lh);
		forceKillNode(lh);
		n = RLP(s);
	}
	assert(n.itemCount() == 2);

	return rlpList(hexPrefixEncode(keyOf(_orig), keyOf(n), isLeaf(n)), n[1]);
//	auto ret =
//	std::cout << keyOf(_orig) << " ++ " << keyOf(n) << " == " << keyOf(RLP(ret)) << std::endl;
//	return ret;
}

template <class DB> bytes GenericTrieDB<DB>::merge(RLP const& _orig, byte _i)
{
#if ETH_PARANOIA
	tdebug << "merge " << _orig << (int)_i;
#endif

	assert(_orig.isList() && _orig.itemCount() == 17);
	RLPStream s(2);
	if (_i != 16)
	{
		assert(!_orig[_i].isEmpty());
		s << hexPrefixEncode(bytesConstRef(&_i, 1), false, 1, 2, 0);
	}
	else
		s << hexPrefixEncode(bytes(), true);
	s << _orig[_i];
	return s.out();
}

template <class DB> bytes GenericTrieDB<DB>::branch(RLP const& _orig)
{
#if ETH_PARANOIA
	tdebug << "branch " << _orig;
#endif

	assert(_orig.isList() && _orig.itemCount() == 2);
	killNode(_orig);

	auto k = keyOf(_orig);
	RLPStream r(17);
	if (k.size() == 0)
	{
		assert(isLeaf(_orig));
		for (unsigned i = 0; i < 16; ++i)
			r << "";
		r << _orig[1];
	}
	else
	{
		byte b = k[0];
		for (unsigned i = 0; i < 16; ++i)
			if (i == b)
				if (isLeaf(_orig) || k.size() > 1)
					streamNode(r, rlpList(hexPrefixEncode(k.mid(1), isLeaf(_orig)), _orig[1]));
				else
					r << _orig[1];
			else
				r << "";
		r << "";
	}
	return r.out();
}

}
