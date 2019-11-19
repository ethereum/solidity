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
/**
 * Visitor templates.
 */

#pragma once

#include <functional>
#include <boost/variant/static_visitor.hpp>

namespace dev
{

/// Generic visitor used as follows:
/// std::visit(GenericVisitor<Class1, Class2>(
///     [](Class1& _c) { _c.f(); },
///     [](Class2& _c) { _c.g(); }
/// ), variant);
/// This one does not have a fallback and will fail at
/// compile-time if you do not specify all variants.

template <class...>
struct GenericVisitor{};

template <class Visitable, class... Others>
struct GenericVisitor<Visitable, Others...>: public GenericVisitor<Others...>
{
	using GenericVisitor<Others...>::operator ();
	explicit GenericVisitor(
		std::function<void(Visitable&)> _visitor,
		std::function<void(Others&)>... _otherVisitors
	):
		GenericVisitor<Others...>(std::move(_otherVisitors)...),
		m_visitor(std::move(_visitor))
	{}

	void operator()(Visitable& _v) const { m_visitor(_v); }

	std::function<void(Visitable&)> m_visitor;
};
template <>
struct GenericVisitor<>: public boost::static_visitor<> {
	void operator()() const {}
};

/// Generic visitor with fallback:
/// std::visit(GenericFallbackVisitor<Class1, Class2>(
///     [](Class1& _c) { _c.f(); },
///     [](Class2& _c) { _c.g(); }
/// ), variant);
/// This one DOES have a fallback and will NOT fail at
/// compile-time if you do not specify all variants.

template <class...>
struct GenericFallbackVisitor{};

template <class Visitable, class... Others>
struct GenericFallbackVisitor<Visitable, Others...>: public GenericFallbackVisitor<Others...>
{
	explicit GenericFallbackVisitor(
		std::function<void(Visitable&)> _visitor,
		std::function<void(Others&)>... _otherVisitors
	):
		GenericFallbackVisitor<Others...>(std::move(_otherVisitors)...),
		m_visitor(std::move(_visitor))
	{}

	using GenericFallbackVisitor<Others...>::operator ();
	void operator()(Visitable& _v) const { m_visitor(_v); }

	std::function<void(Visitable&)> m_visitor;
};
template <>
struct GenericFallbackVisitor<>: public boost::static_visitor<> {
	template <class T>
	void operator()(T&) const { }
};

/// Generic visitor with fallback that can return a value:
/// std::visit(GenericFallbackReturnsVisitor<ReturnType, Class1, Class2>(
///     [](Class1& _c) { return _c.f(); },
///     [](Class2& _c) { return _c.g(); }
/// ), variant);
/// This one DOES have a fallback and will NOT fail at
/// compile-time if you do not specify all variants.
/// The fallback {}-constructs the return value.

template <class R, class...>
struct GenericFallbackReturnsVisitor{};

template <class R, class Visitable, class... Others>
struct GenericFallbackReturnsVisitor<R, Visitable, Others...>: public GenericFallbackReturnsVisitor<R, Others...>
{
	explicit GenericFallbackReturnsVisitor(
		std::function<R(Visitable&)> _visitor,
		std::function<R(Others&)>... _otherVisitors
	):
		GenericFallbackReturnsVisitor<R, Others...>(std::move(_otherVisitors)...),
		m_visitor(std::move(_visitor))
	{}

	using GenericFallbackReturnsVisitor<R, Others...>::operator ();
	R operator()(Visitable& _v) const { return m_visitor(_v); }

	std::function<R(Visitable&)> m_visitor;
};
template <class R>
struct GenericFallbackReturnsVisitor<R>: public boost::static_visitor<R> {
	template <class T>
	R operator()(T&) const { return {}; }
};

}
