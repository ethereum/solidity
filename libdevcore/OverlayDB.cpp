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
/** @file OverlayDB.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */
#if !defined(ETH_EMSCRIPTEN)

#include <thread>
#include <libdevcore/db.h>
#include <libdevcore/Common.h>
#include "OverlayDB.h"
using namespace std;
using namespace dev;

namespace dev
{

h256 const EmptyTrie = sha3(rlp(""));

OverlayDB::~OverlayDB()
{
	if (m_db.use_count() == 1 && m_db.get())
		ctrace << "Closing state DB";
}

class WriteBatchNoter: public ldb::WriteBatch::Handler
{
	virtual void Put(ldb::Slice const& _key, ldb::Slice const& _value) { cnote << "Put" << toHex(bytesConstRef(_key)) << "=>" << toHex(bytesConstRef(_value)); }
	virtual void Delete(ldb::Slice const& _key) { cnote << "Delete" << toHex(bytesConstRef(_key)); }
};

void OverlayDB::commit()
{
	if (m_db)
	{
		ldb::WriteBatch batch;
//		cnote << "Committing nodes to disk DB:";
#if DEV_GUARDED_DB
		DEV_READ_GUARDED(x_this)
#endif
		{
			for (auto const& i: m_main)
			{
				if (i.second.second)
					batch.Put(ldb::Slice((char const*)i.first.data(), i.first.size), ldb::Slice(i.second.first.data(), i.second.first.size()));
//				cnote << i.first << "#" << m_main[i.first].second;
			}
			for (auto const& i: m_aux)
				if (i.second.second)
				{
					bytes b = i.first.asBytes();
					b.push_back(255);	// for aux
					batch.Put(bytesConstRef(&b), bytesConstRef(&i.second.first));
				}
		}

		for (unsigned i = 0; i < 10; ++i)
		{
			ldb::Status o = m_db->Write(m_writeOptions, &batch);
			if (o.ok())
				break;
			if (i == 9)
			{
				cwarn << "Fail writing to state database. Bombing out.";
				exit(-1);
			}
			cwarn << "Error writing to state database: " << o.ToString();
			WriteBatchNoter n;
			batch.Iterate(&n);
			cwarn << "Sleeping for" << (i + 1) << "seconds, then retrying.";
			this_thread::sleep_for(chrono::seconds(i + 1));
		}
#if DEV_GUARDED_DB
		DEV_WRITE_GUARDED(x_this)
#endif
		{
			m_aux.clear();
			m_main.clear();
		}
	}
}

bytes OverlayDB::lookupAux(h256 const& _h) const
{
	bytes ret = MemoryDB::lookupAux(_h);
	if (!ret.empty() || !m_db)
		return ret;
	std::string v;
	bytes b = _h.asBytes();
	b.push_back(255);	// for aux
	m_db->Get(m_readOptions, bytesConstRef(&b), &v);
	if (v.empty())
		cwarn << "Aux not found: " << _h;
	return asBytes(v);
}

void OverlayDB::rollback()
{
#if DEV_GUARDED_DB
	WriteGuard l(x_this);
#endif
	m_main.clear();
}

std::string OverlayDB::lookup(h256 const& _h) const
{
	std::string ret = MemoryDB::lookup(_h);
	if (ret.empty() && m_db)
		m_db->Get(m_readOptions, ldb::Slice((char const*)_h.data(), 32), &ret);
	return ret;
}

bool OverlayDB::exists(h256 const& _h) const
{
	if (MemoryDB::exists(_h))
		return true;
	std::string ret;
	if (m_db)
		m_db->Get(m_readOptions, ldb::Slice((char const*)_h.data(), 32), &ret);
	return !ret.empty();
}

void OverlayDB::kill(h256 const& _h)
{
#if ETH_PARANOIA || 1
	if (!MemoryDB::kill(_h))
	{
		std::string ret;
		if (m_db)
			m_db->Get(m_readOptions, ldb::Slice((char const*)_h.data(), 32), &ret);
		// No point node ref decreasing for EmptyTrie since we never bother incrementing it in the first place for
		// empty storage tries.
		if (ret.empty() && _h != EmptyTrie)
			cnote << "Decreasing DB node ref count below zero with no DB node. Probably have a corrupt Trie." << _h;

		// TODO: for 1.1: ref-counted triedb.
	}
#else
	MemoryDB::kill(_h);
#endif
}

bool OverlayDB::deepkill(h256 const& _h)
{
	// kill in memoryDB
	kill(_h);

	//kill in overlayDB
	ldb::Status s = m_db->Delete(m_writeOptions, ldb::Slice((char const*)_h.data(), 32));
	if (s.ok())
		return true;
	else
		return false;
}

}

#endif // ETH_EMSCRIPTEN
