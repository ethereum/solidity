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

namespace solidity::util
{

/**
 * Generic visitor used as follows:
 * std::visit(GenericVisitor{
 *     [](Class1& _c) { _c.f(); },
 *     [](Class2& _c) { _c.g(); }
 * }, variant);
 * This one does not have a fallback and will fail at
 * compile-time if you do not specify all variants.
 *
 * Fallback with no return (it will not fail if you do not specify all variants):
 * std::visit(GenericVisitor{
 *     VisitorFallback<>{},
 *     [](Class1& _c) { _c.f(); },
 *     [](Class2& _c) { _c.g(); }
 * }, variant);
 *
 * Fallback with return type R (the fallback returns `R{}`:
 * std::visit(GenericVisitor{
 *     VisitorFallback<R>{},
 *     [](Class1& _c) { _c.f(); },
 *     [](Class2& _c) { _c.g(); }
 * }, variant);
 */

template <typename...> struct VisitorFallback;

template <typename R>
struct VisitorFallback<R> { template<typename T> R operator()(T&&) const { return {}; } };

template<>
struct VisitorFallback<> { template<typename T> void operator()(T&&) const {} };

template <typename... Visitors> struct GenericVisitor: Visitors... { using Visitors::operator()...; };
template <typename... Visitors> GenericVisitor(Visitors...) -> GenericVisitor<Visitors...>;
}
