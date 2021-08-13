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
// SPDX-License-Identifier: GPL-3.0
/** @file CommonData.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Shared algorithms and data types.
 */

#pragma once

#include <iterator>
#include <libsolutil/Common.h>

#include <vector>
#include <type_traits>
#include <cstring>
#include <optional>
#include <string>
#include <set>
#include <functional>
#include <utility>
#include <type_traits>

/// Operators need to stay in the global namespace.

/// Concatenate the contents of a container onto a vector
template <class T, class U> std::vector<T>& operator+=(std::vector<T>& _a, U& _b)
{
	for (auto const& i: _b)
		_a.push_back(T(i));
	return _a;
}
/// Concatenate the contents of a container onto a vector, move variant.
template <class T, class U> std::vector<T>& operator+=(std::vector<T>& _a, U&& _b)
{
	std::move(_b.begin(), _b.end(), std::back_inserter(_a));
	return _a;
}
/// Concatenate the contents of a container onto a multiset
template <class U, class... T> std::multiset<T...>& operator+=(std::multiset<T...>& _a, U& _b)
{
	_a.insert(_b.begin(), _b.end());
	return _a;
}
/// Concatenate the contents of a container onto a multiset, move variant.
template <class U, class... T> std::multiset<T...>& operator+=(std::multiset<T...>& _a, U&& _b)
{
	for (auto&& x: _b)
		_a.insert(std::move(x));
	return _a;
}
/// Concatenate the contents of a container onto a set
template <class U, class... T> std::set<T...>& operator+=(std::set<T...>& _a, U& _b)
{
	_a.insert(_b.begin(), _b.end());
	return _a;
}
/// Concatenate the contents of a container onto a set, move variant.
template <class U, class... T> std::set<T...>& operator+=(std::set<T...>& _a, U&& _b)
{
	for (auto&& x: _b)
		_a.insert(std::move(x));
	return _a;
}

/// Concatenate two vectors of elements.
template <class T>
inline std::vector<T> operator+(std::vector<T> const& _a, std::vector<T> const& _b)
{
	std::vector<T> ret(_a);
	ret += _b;
	return ret;
}

/// Concatenate two vectors of elements, moving them.
template <class T>
inline std::vector<T> operator+(std::vector<T>&& _a, std::vector<T>&& _b)
{
	std::vector<T> ret(std::move(_a));
	assert(&_a != &_b);
	ret += std::move(_b);
	return ret;
}

/// Concatenate something to a sets of elements.
template <class U, class... T>
inline std::set<T...> operator+(std::set<T...> const& _a, U&& _b)
{
	std::set<T...> ret(_a);
	ret += std::forward<U>(_b);
	return ret;
}

/// Concatenate something to a sets of elements, move variant.
template <class U, class... T>
inline std::set<T...> operator+(std::set<T...>&& _a, U&& _b)
{
	std::set<T...> ret(std::move(_a));
	ret += std::forward<U>(_b);
	return ret;
}

/// Remove the elements of a container from a set.
template <class C, class... T>
inline std::set<T...>& operator-=(std::set<T...>& _a, C const& _b)
{
	for (auto const& x: _b)
		_a.erase(x);
	return _a;
}

template <class C, class... T>
inline std::set<T...> operator-(std::set<T...> const& _a, C const& _b)
{
	auto result = _a;
	result -= _b;
	return result;
}

/// Remove the elements of a container from a multiset.
template <class C, class... T>
inline std::multiset<T...>& operator-=(std::multiset<T...>& _a, C const& _b)
{
	for (auto const& x: _b)
		_a.erase(x);
	return _a;
}

namespace solidity::util
{

/// Functional map.
/// Returns a container _oc applying @param _op to each element in @param _c.
/// By default _oc is a vector.
/// If another return type is desired, an empty contained of that type
/// is given as @param _oc.
template<class Container, class Callable, class OutputContainer =
	std::vector<std::invoke_result_t<
		Callable,
		decltype(*std::begin(std::declval<Container>()))
>>>
auto applyMap(Container const& _c, Callable&& _op, OutputContainer _oc = OutputContainer{})
{
	std::transform(std::begin(_c), std::end(_c), std::inserter(_oc, std::end(_oc)), _op);
	return _oc;
}

/// Filter a vector.
/// Returns a copy of the vector after only taking indices `i` such that `_mask[i]` is true.
template<typename T>
std::vector<T> filter(std::vector<T> const& _vec, std::vector<bool> const& _mask)
{
	assert(_vec.size() == _mask.size());

	std::vector<T> ret;

	for (size_t i = 0; i < _mask.size(); ++i)
		if (_mask[i])
			ret.push_back(_vec[i]);

	return ret;
}

/// Functional fold.
/// Given a container @param _c, an initial value @param _acc,
/// and a binary operator @param _binaryOp(T, U), accumulate
/// the elements of _c over _acc.
/// Note that <numeric> has a similar function `accumulate` which
/// until C++20 does *not* std::move the partial accumulated.
template<class C, class T, class Callable>
auto fold(C const& _c, T _acc, Callable&& _binaryOp)
{
	for (auto const& e: _c)
		_acc = _binaryOp(std::move(_acc), e);
	return _acc;
}

template <class T, class U>
T convertContainer(U const& _from)
{
	return T{_from.cbegin(), _from.cend()};
}

template <class T, class U>
T convertContainer(U&& _from)
{
	return T{
		std::make_move_iterator(_from.begin()),
		std::make_move_iterator(_from.end())
	};
}

/// Gets a @a K -> @a V map and returns a map where values from the original map are keys and keys
/// from the original map are values.
///
/// @pre @a originalMap must have unique values.
template <typename K, typename V>
std::map<V, K> invertMap(std::map<K, V> const& originalMap)
{
	std::map<V, K> inverseMap;
	for (auto const& originalPair: originalMap)
	{
		assert(inverseMap.count(originalPair.second) == 0);
		inverseMap.insert({originalPair.second, originalPair.first});
	}

	return inverseMap;
}

/// Returns a set of keys of a map.
template <typename K, typename V>
std::set<K> keys(std::map<K, V> const& _map)
{
	return applyMap(_map, [](auto const& _elem) { return _elem.first; }, std::set<K>{});
}

/// @returns a pointer to the entry of @a _map at @a _key, if there is one, and nullptr otherwise.
template<typename MapType, typename KeyType>
decltype(auto) valueOrNullptr(MapType&& _map, KeyType const& _key)
{
	auto it = _map.find(_key);
	return (it == _map.end()) ? nullptr : &it->second;
}

namespace detail
{
struct allow_copy {};
}
static constexpr auto allow_copy = detail::allow_copy{};

/// @returns a reference to the entry of @a _map at @a _key, if there is one, and @a _defaultValue otherwise.
/// Makes sure no copy is involved, unless allow_copy is passed as fourth argument.
template<
	typename MapType,
	typename KeyType,
	typename ValueType = std::decay_t<decltype(std::declval<MapType>().find(std::declval<KeyType>())->second)> const&,
	typename AllowCopyType = void*
>
decltype(auto) valueOrDefault(MapType&& _map, KeyType const& _key, ValueType&& _defaultValue = {}, AllowCopyType = nullptr)
{
	auto it = _map.find(_key);
	static_assert(
		std::is_same_v<AllowCopyType, detail::allow_copy> ||
		std::is_reference_v<decltype((it == _map.end()) ? _defaultValue : it->second)>,
		"valueOrDefault does not allow copies by default. Pass allow_copy as additional argument, if you want to allow copies."
	);
	return (it == _map.end()) ? _defaultValue : it->second;
}

namespace detail
{
template<typename Callable>
struct MapTuple
{
	Callable callable;
	template<typename TupleType>
	decltype(auto) operator()(TupleType&& _tuple) {
		using PlainTupleType = std::remove_cv_t<std::remove_reference_t<TupleType>>;
		return operator()(
			std::forward<TupleType>(_tuple),
			std::make_index_sequence<std::tuple_size_v<PlainTupleType>>{}
		);
	}
private:
	template<typename TupleType, size_t... I>
	decltype(auto) operator()(TupleType&& _tuple, std::index_sequence<I...>)
	{
		return callable(std::get<I>(std::forward<TupleType>(_tuple))...);
	}
};
}

/// Wraps @a _callable, which takes multiple arguments, into a callable that takes a single tuple of arguments.
/// Since structured binding in lambdas is not allowed, i.e. [](auto&& [key, value]) { ... } is invalid, this allows
/// to instead use mapTuple([](auto&& key, auto&& value) { ... }).
template<typename Callable>
decltype(auto) mapTuple(Callable&& _callable)
{
	return detail::MapTuple<Callable>{std::forward<Callable>(_callable)};
}

/// Merges map @a _b into map @a _a. If the same key exists in both maps,
/// calls @a _conflictSolver to combine the two values.
template <class K, class V, class F>
void joinMap(std::map<K, V>& _a, std::map<K, V>&& _b, F _conflictSolver)
{
	auto ita = _a.begin();
	auto aend = _a.end();
	auto itb = _b.begin();
	auto bend = _b.end();

	for (; itb != bend; ++ita)
	{
		if (ita == aend)
			ita = _a.insert(ita, std::move(*itb++));
		else if (ita->first < itb->first)
			continue;
		else if (itb->first < ita->first)
			ita = _a.insert(ita, std::move(*itb++));
		else
		{
			_conflictSolver(ita->second, std::move(itb->second));
			++itb;
		}
	}
}

// String conversion functions, mainly to/from hex/nibble/byte representations.

enum class WhenError
{
	DontThrow = 0,
	Throw = 1,
};

enum class HexPrefix
{
	DontAdd = 0,
	Add = 1,
};

enum class HexCase
{
	Lower = 0,
	Upper = 1,
	Mixed = 2,
};

/// Convert a single byte to a string of hex characters (of length two),
/// optionally with uppercase hex letters.
std::string toHex(uint8_t _data, HexCase _case = HexCase::Lower);

/// Convert a series of bytes to the corresponding string of hex duplets,
/// optionally with "0x" prefix and with uppercase hex letters.
std::string toHex(bytes const& _data, HexPrefix _prefix = HexPrefix::DontAdd, HexCase _case = HexCase::Lower);

/// Converts a (printable) ASCII hex character into the corresponding integer value.
/// @example fromHex('A') == 10 && fromHex('f') == 15 && fromHex('5') == 5
int fromHex(char _i, WhenError _throw);

/// Converts a (printable) ASCII hex string into the corresponding byte stream.
/// @example fromHex("41626261") == asBytes("Abba")
/// If _throw = ThrowType::DontThrow, it replaces bad hex characters with 0's, otherwise it will throw an exception.
bytes fromHex(std::string const& _s, WhenError _throw = WhenError::DontThrow);
/// Converts byte array to a string containing the same (binary) data. Unless
/// the byte array happens to contain ASCII data, this won't be printable.
inline std::string asString(bytes const& _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}

/// Converts byte array ref to a string containing the same (binary) data. Unless
/// the byte array happens to contain ASCII data, this won't be printable.
inline std::string asString(bytesConstRef _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}

/// Converts a string to a byte array containing the string's (byte) data.
inline bytes asBytes(std::string const& _b)
{
	return bytes((uint8_t const*)_b.data(), (uint8_t const*)(_b.data() + _b.size()));
}

// Big-endian to/from host endian conversion functions.

/// Converts a templated integer value to the big-endian byte-stream represented on a templated collection.
/// The size of the collection object will be unchanged. If it is too small, it will not represent the
/// value properly, if too big then the additional elements will be zeroed out.
/// @a Out will typically be either std::string or bytes.
/// @a T will typically by unsigned, u160, u256 or bigint.
template <class T, class Out>
inline void toBigEndian(T _val, Out& o_out)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	for (auto i = o_out.size(); i != 0; _val >>= 8, i--)
	{
		T v = _val & (T)0xff;
		o_out[i - 1] = (typename Out::value_type)(uint8_t)v;
	}
}

/// Converts a big-endian byte-stream represented on a templated collection to a templated integer value.
/// @a In will typically be either std::string or bytes.
/// @a T will typically by unsigned, u256 or bigint.
template <class T, class In>
inline T fromBigEndian(In const& _bytes)
{
	T ret = (T)0;
	for (auto i: _bytes)
		ret = (T)((ret << 8) | (uint8_t)(typename std::make_unsigned<typename In::value_type>::type)i);
	return ret;
}
inline bytes toBigEndian(u256 _val) { bytes ret(32); toBigEndian(_val, ret); return ret; }

/// Convenience function for toBigEndian.
/// @returns a byte array just big enough to represent @a _val.
template <class T>
inline bytes toCompactBigEndian(T _val, unsigned _min = 0)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	unsigned i = 0;
	for (T v = _val; v; ++i, v >>= 8) {}
	bytes ret(std::max<unsigned>(_min, i), 0);
	toBigEndian(_val, ret);
	return ret;
}

/// Convenience function for conversion of a u256 to hex
inline std::string toHex(u256 val, HexPrefix prefix = HexPrefix::DontAdd)
{
	std::string str = toHex(toBigEndian(val));
	return (prefix == HexPrefix::Add) ? "0x" + str : str;
}

template <class T>
inline std::string toCompactHexWithPrefix(T _value)
{
	return toHex(toCompactBigEndian(_value, 1), HexPrefix::Add);
}

/// Returns decimal representation for small numbers and hex for large numbers.
inline std::string formatNumber(bigint const& _value)
{
	if (_value < 0)
		return "-" + formatNumber(-_value);
	if (_value > 0x1000000)
		return toHex(toCompactBigEndian(_value, 1), HexPrefix::Add);
	else
		return _value.str();
}

inline std::string formatNumber(u256 const& _value)
{
	if (_value > 0x1000000)
		return toCompactHexWithPrefix(_value);
	else
		return _value.str();
}


// Algorithms for string and string-like collections.

/// Determine bytes required to encode the given integer value. @returns 0 if @a _i is zero.
template <class T>
inline unsigned bytesRequired(T _i)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	unsigned i = 0;
	for (; _i != 0; ++i, _i >>= 8) {}
	return i;
}
template <class T, class V>
bool contains(T const& _t, V const& _v)
{
	return std::end(_t) != std::find(std::begin(_t), std::end(_t), _v);
}

template <class T, class Predicate>
bool contains_if(T const& _t, Predicate const& _p)
{
	return std::end(_t) != std::find_if(std::begin(_t), std::end(_t), _p);
}

/// Function that iterates over a vector, calling a function on each of its
/// elements. If that function returns a vector, the element is replaced by
/// the returned vector. During the iteration, the original vector is only valid
/// on the current element and after that. The actual replacement takes
/// place at the end, but already visited elements might be invalidated.
/// If nothing is replaced, no copy is performed.
template <typename T, typename F>
void iterateReplacing(std::vector<T>& _vector, F const& _f)
{
	// Concept: _f must be Callable, must accept param T&, must return optional<vector<T>>
	bool useModified = false;
	std::vector<T> modifiedVector;
	for (size_t i = 0; i < _vector.size(); ++i)
	{
		if (std::optional<std::vector<T>> r = _f(_vector[i]))
		{
			if (!useModified)
			{
				std::move(_vector.begin(), _vector.begin() + ptrdiff_t(i), back_inserter(modifiedVector));
				useModified = true;
			}
			modifiedVector += std::move(*r);
		}
		else if (useModified)
			modifiedVector.emplace_back(std::move(_vector[i]));
	}
	if (useModified)
		_vector = std::move(modifiedVector);
}

namespace detail
{
template <typename T, typename F, std::size_t... I>
void iterateReplacingWindow(std::vector<T>& _vector, F const& _f, std::index_sequence<I...>)
{
	// Concept: _f must be Callable, must accept sizeof...(I) parameters of type T&, must return optional<vector<T>>
	bool useModified = false;
	std::vector<T> modifiedVector;
	size_t i = 0;
	for (; i + sizeof...(I) <= _vector.size(); ++i)
	{
		if (std::optional<std::vector<T>> r = _f(_vector[i + I]...))
		{
			if (!useModified)
			{
				std::move(_vector.begin(), _vector.begin() + ptrdiff_t(i), back_inserter(modifiedVector));
				useModified = true;
			}
			modifiedVector += std::move(*r);
			i += sizeof...(I) - 1;
		}
		else if (useModified)
			modifiedVector.emplace_back(std::move(_vector[i]));
	}
	if (useModified)
	{
		for (; i < _vector.size(); ++i)
			modifiedVector.emplace_back(std::move(_vector[i]));
		_vector = std::move(modifiedVector);
	}
}

}

/// Function that iterates over the vector @param _vector,
/// calling the function @param _f on sequences of @tparam N of its
/// elements. If @param _f returns a vector, these elements are replaced by
/// the returned vector and the iteration continues with the next @tparam N elements.
/// If the function does not return a vector, the iteration continues with an overlapping
/// sequence of @tparam N elements that starts with the second element of the previous
/// iteration.
/// During the iteration, the original vector is only valid
/// on the current element and after that. The actual replacement takes
/// place at the end, but already visited elements might be invalidated.
/// If nothing is replaced, no copy is performed.
template <std::size_t N, typename T, typename F>
void iterateReplacingWindow(std::vector<T>& _vector, F const& _f)
{
	// Concept: _f must be Callable, must accept N parameters of type T&, must return optional<vector<T>>
	detail::iterateReplacingWindow(_vector, _f, std::make_index_sequence<N>{});
}

/// @returns true iff @a _str passess the hex address checksum test.
/// @param _strict if false, hex strings with only uppercase or only lowercase letters
/// are considered valid.
bool passesAddressChecksum(std::string const& _str, bool _strict);

/// @returns the checksummed version of an address
/// @param hex strings that look like an address
std::string getChecksummedAddress(std::string const& _addr);

bool isValidHex(std::string const& _string);
bool isValidDecimal(std::string const& _string);

/// @returns a quoted string if all characters are printable ASCII chars,
/// or its hex representation otherwise.
/// _value cannot be longer than 32 bytes.
std::string formatAsStringOrNumber(std::string const& _value);

/// @returns a string with the usual backslash-escapes for non-printable and non-ASCII
/// characters and surrounded by '"'-characters.
std::string escapeAndQuoteString(std::string const& _input);

template<typename Container, typename Compare>
bool containerEqual(Container const& _lhs, Container const& _rhs, Compare&& _compare)
{
	return std::equal(std::begin(_lhs), std::end(_lhs), std::begin(_rhs), std::end(_rhs), std::forward<Compare>(_compare));
}

inline std::string findAnyOf(std::string const& _haystack, std::vector<std::string> const& _needles)
{
	for (std::string const& needle: _needles)
		if (_haystack.find(needle) != std::string::npos)
			return needle;
	return "";
}


namespace detail
{
template<typename T>
void variadicEmplaceBack(std::vector<T>&) {}
template<typename T, typename A, typename... Args>
void variadicEmplaceBack(std::vector<T>& _vector, A&& _a, Args&&... _args)
{
	_vector.emplace_back(std::forward<A>(_a));
	variadicEmplaceBack(_vector, std::forward<Args>(_args)...);
}
}

template<typename T, typename... Args>
std::vector<T> make_vector(Args&&... _args)
{
	std::vector<T> result;
	result.reserve(sizeof...(_args));
	detail::variadicEmplaceBack(result, std::forward<Args>(_args)...);
	return result;
}

}
