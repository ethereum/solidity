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
/** @file ABI.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/SHA3.h>

namespace dev
{
namespace eth
{

inline string32 toString32(std::string const& _s)
{
	string32 ret;
	for (unsigned i = 0; i < 32; ++i)
		ret[i] = i < _s.size() ? _s[i] : 0;
	return ret;
}

template <class T> struct ABISerialiser {};
template <unsigned N> struct ABISerialiser<FixedHash<N>> { static bytes serialise(FixedHash<N> const& _t) { static_assert(N <= 32, "Cannot serialise hash > 32 bytes."); static_assert(N > 0, "Cannot serialise zero-length hash."); return bytes(32 - N, 0) + _t.asBytes(); } };
template <> struct ABISerialiser<u256> { static bytes serialise(u256 const& _t) { return h256(_t).asBytes(); } };
template <> struct ABISerialiser<u160> { static bytes serialise(u160 const& _t) { return bytes(12, 0) + h160(_t).asBytes(); } };
template <> struct ABISerialiser<string32> { static bytes serialise(string32 const& _t) { bytes ret; bytesConstRef((byte const*)_t.data(), 32).populate(bytesRef(&ret)); return ret; } };
template <> struct ABISerialiser<std::string>
{
	static bytes serialise(std::string const& _t)
	{
		bytes ret = h256(u256(32)).asBytes() + h256(u256(_t.size())).asBytes();
		ret.resize(ret.size() + (_t.size() + 31) / 32 * 32);
		bytesConstRef(&_t).populate(bytesRef(&ret).cropped(64));
		return ret;
	}
};

inline bytes abiInAux() { return {}; }
template <class T, class ... U> bytes abiInAux(T const& _t, U const& ... _u)
{
	return ABISerialiser<T>::serialise(_t) + abiInAux(_u ...);
}

template <class ... T> bytes abiIn(std::string _id, T const& ... _t)
{
	return keccak256(_id).ref().cropped(0, 4).toBytes() + abiInAux(_t ...);
}

template <class T> struct ABIDeserialiser {};
template <unsigned N> struct ABIDeserialiser<FixedHash<N>> { static FixedHash<N> deserialise(bytesConstRef& io_t) { static_assert(N <= 32, "Parameter sizes must be at most 32 bytes."); FixedHash<N> ret; io_t.cropped(32 - N, N).populate(ret.ref()); io_t = io_t.cropped(32); return ret; } };
template <> struct ABIDeserialiser<u256> { static u256 deserialise(bytesConstRef& io_t) { u256 ret = fromBigEndian<u256>(io_t.cropped(0, 32)); io_t = io_t.cropped(32); return ret; } };
template <> struct ABIDeserialiser<u160> { static u160 deserialise(bytesConstRef& io_t) { u160 ret = fromBigEndian<u160>(io_t.cropped(12, 20)); io_t = io_t.cropped(32); return ret; } };
template <> struct ABIDeserialiser<string32> { static string32 deserialise(bytesConstRef& io_t) { string32 ret; io_t.cropped(0, 32).populate(bytesRef((byte*)ret.data(), 32)); io_t = io_t.cropped(32); return ret; } };
template <> struct ABIDeserialiser<std::string>
{
	static std::string deserialise(bytesConstRef& io_t)
	{
		unsigned o = (uint16_t)u256(h256(io_t.cropped(0, 32)));
		unsigned s = (uint16_t)u256(h256(io_t.cropped(o, 32)));
		std::string ret;
		ret.resize(s);
		io_t.cropped(o + 32, s).populate(bytesRef((byte*)ret.data(), s));
		io_t = io_t.cropped(32);
		return ret;
	}
};

template <class T> T abiOut(bytes const& _data)
{
	bytesConstRef o(&_data);
	return ABIDeserialiser<T>::deserialise(o);
}

template <class T> T abiOut(bytesConstRef& _data)
{
	return ABIDeserialiser<T>::deserialise(_data);
}

}
}
