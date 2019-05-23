#pragma once

#include <cstring>
#include <cassert>
#include <type_traits>
#include <vector>
#include <string>

#ifdef __INTEL_COMPILER
#pragma warning(disable:597) // will not be called for implicit or explicit conversions
#endif

namespace dev
{

/**
 * A modifiable reference to an existing object or vector in memory.
 */
template <class _T>
class vector_ref
{
public:
	using value_type = _T;
	using element_type = _T;
	using mutable_value_type = typename std::conditional<std::is_const<_T>::value, typename std::remove_const<_T>::type, _T>::type;
	using string_type = typename std::conditional<std::is_const<_T>::value, std::string const, std::string>::type;
	using vector_type = typename std::conditional<std::is_const<_T>::value, std::vector<typename std::remove_const<_T>::type> const, std::vector<_T>>::type;
	using iterator = _T*;
	using const_iterator = _T const*;

	static_assert(std::is_pod<value_type>::value, "vector_ref can only be used with PODs due to its low-level treatment of data.");

	vector_ref(): m_data(nullptr), m_count(0) {}
	/// Creates a new vector_ref to point to @a _count elements starting at @a _data.
	vector_ref(_T* _data, size_t _count): m_data(_data), m_count(_count) {}
	/// Creates a new vector_ref pointing to the data part of a string (given as pointer).
	vector_ref(string_type* _data): m_data(reinterpret_cast<_T*>(_data->data())), m_count(_data->size() / sizeof(_T)) {}
	/// Creates a new vector_ref pointing to the data part of a string (given as reference).
	vector_ref(string_type& _data): vector_ref(&_data) {}
	/// Creates a new vector_ref pointing to the data part of a vector (given as pointer).
	vector_ref(vector_type* _data): m_data(_data->data()), m_count(_data->size()) {}
	explicit operator bool() const { return m_data && m_count; }

	std::vector<unsigned char> toBytes() const { return std::vector<unsigned char>(reinterpret_cast<unsigned char const*>(m_data), reinterpret_cast<unsigned char const*>(m_data) + m_count * sizeof(_T)); }
	std::string toString() const { return std::string((char const*)m_data, ((char const*)m_data) + m_count * sizeof(_T)); }

	template <class _T2> explicit operator vector_ref<_T2>() const { assert(m_count * sizeof(_T) / sizeof(_T2) * sizeof(_T2) / sizeof(_T) == m_count); return vector_ref<_T2>(reinterpret_cast<_T2*>(m_data), m_count * sizeof(_T) / sizeof(_T2)); }
	operator vector_ref<_T const>() const { return vector_ref<_T const>(m_data, m_count); }

	_T* data() const { return m_data; }
	/// @returns the number of elements referenced (not necessarily number of bytes).
	size_t size() const { return m_count; }
	bool empty() const { return !m_count; }
	/// @returns a new vector_ref which is a shifted and shortened view of the original data.
	/// If this goes out of bounds in any way, returns an empty vector_ref.
	/// If @a _count is ~size_t(0), extends the view to the end of the data.
	vector_ref<_T> cropped(size_t _begin, size_t _count) const { if (m_data && _begin <= m_count && _count <= m_count && _begin + _count <= m_count) return vector_ref<_T>(m_data + _begin, _count == ~size_t(0) ? m_count - _begin : _count); else return vector_ref<_T>(); }
	/// @returns a new vector_ref which is a shifted view of the original data (not going beyond it).
	vector_ref<_T> cropped(size_t _begin) const { if (m_data && _begin <= m_count) return vector_ref<_T>(m_data + _begin, m_count - _begin); else return vector_ref<_T>(); }

	_T* begin() { return m_data; }
	_T* end() { return m_data + m_count; }
	_T const* begin() const { return m_data; }
	_T const* end() const { return m_data + m_count; }

	_T& operator[](size_t _i) { assert(m_data); assert(_i < m_count); return m_data[_i]; }
	_T const& operator[](size_t _i) const { assert(m_data); assert(_i < m_count); return m_data[_i]; }

	bool operator==(vector_ref<_T> const& _cmp) const { return m_data == _cmp.m_data && m_count == _cmp.m_count; }
	bool operator!=(vector_ref<_T> const& _cmp) const { return !operator==(_cmp); }

	void reset() { m_data = nullptr; m_count = 0; }

private:
	_T* m_data = nullptr;
	size_t m_count = 0;
};

}
