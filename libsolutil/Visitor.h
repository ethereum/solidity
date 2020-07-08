// SPDX-License-Identifier: GPL-3.0
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
