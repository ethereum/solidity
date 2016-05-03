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
/** @file RLP.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "RLP.h"
using namespace std;
using namespace dev;

RLPStream& RLPStream::appendRaw(bytesConstRef _s, size_t _itemCount)
{
	m_out.insert(m_out.end(), _s.begin(), _s.end());
	noteAppended(_itemCount);
	return *this;
}

void RLPStream::noteAppended(size_t _itemCount)
{
	if (!_itemCount)
		return;
//	cdebug << "noteAppended(" << _itemCount << ")";
	while (m_listStack.size())
	{
		if (m_listStack.back().first < _itemCount)
			BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("itemCount too large") << RequirementError((bigint)m_listStack.back().first, (bigint)_itemCount));
		m_listStack.back().first -= _itemCount;
		if (m_listStack.back().first)
			break;
		else
		{
			auto p = m_listStack.back().second;
			m_listStack.pop_back();
			size_t s = m_out.size() - p;		// list size
			auto brs = bytesRequired(s);
			unsigned encodeSize = s < c_rlpListImmLenCount ? 1 : (1 + brs);
//			cdebug << "s: " << s << ", p: " << p << ", m_out.size(): " << m_out.size() << ", encodeSize: " << encodeSize << " (br: " << brs << ")";
			auto os = m_out.size();
			m_out.resize(os + encodeSize);
			memmove(m_out.data() + p + encodeSize, m_out.data() + p, os - p);
			if (s < c_rlpListImmLenCount)
				m_out[p] = (byte)(c_rlpListStart + s);
			else if (c_rlpListIndLenZero + brs <= 0xff)
			{
				m_out[p] = (byte)(c_rlpListIndLenZero + brs);
				byte* b = &(m_out[p + brs]);
				for (; s; s >>= 8)
					*(b--) = (byte)s;
			}
			else
				BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("itemCount too large for RLP"));
		}
		_itemCount = 1;	// for all following iterations, we've effectively appended a single item only since we completed a list.
	}
}

RLPStream& RLPStream::appendList(size_t _items)
{
//	cdebug << "appendList(" << _items << ")";
	if (_items)
		m_listStack.push_back(std::make_pair(_items, m_out.size()));
	else
		appendList(bytes());
	return *this;
}

RLPStream& RLPStream::appendList(bytesConstRef _rlp)
{
	if (_rlp.size() < c_rlpListImmLenCount)
		m_out.push_back((byte)(_rlp.size() + c_rlpListStart));
	else
		pushCount(_rlp.size(), c_rlpListIndLenZero);
	appendRaw(_rlp, 1);
	return *this;
}

void RLPStream::pushCount(size_t _count, byte _base)
{
	auto br = bytesRequired(_count);
	if (int(br) + _base > 0xff)
		BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("Count too large for RLP"));
	m_out.push_back((byte)(br + _base));	// max 8 bytes.
	pushInt(_count, br);
}
