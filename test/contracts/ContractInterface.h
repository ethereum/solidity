/*
    This file is part of solidity.
    solidity is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    solidity is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with solidity. If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <boost/test/unit_test.hpp>
#include <test/ExecutionFramework.h>
#include <functional>

namespace solidity::test
{
class ContractInterface
{
public:
    // Constructor to initialize the ExecutionFramework reference
    ContractInterface(ExecutionFramework& framework)
        : m_framework(framework), m_nextValue(0) {}

    // Set the next value for contract calls
    void setNextValue(const u256& value)
    {
        m_nextValue = value;
    }

protected:
    // Template method to call a contract function
    template <class... Args>
    const bytes& call(const std::string& sig, Args const&... arguments)
    {
        auto const& ret = m_framework.callContractFunctionWithValue(sig, m_nextValue, arguments...);
        m_nextValue = 0;  // Reset next value after the call
        return ret;
    }

    // Call a contract function that accepts a string argument
    void callString(const std::string& name, const std::string& arg)
    {
        BOOST_CHECK(call(name + "(string)", u256(0x20), arg.length(), arg).empty());
    }

    // Call a contract function with a string and an address
    void callStringAddress(const std::string& name, const std::string& arg1, const util::h160& arg2)
    {
        BOOST_CHECK(call(name + "(string,address)", u256(0x40), arg2, arg1.length(), arg1).empty());
    }

    // Call a contract function with a string, an address, and a boolean
    void callStringAddressBool(const std::string& name, const std::string& arg1, const util::h160& arg2, bool arg3)
    {
        BOOST_CHECK(call(name + "(string,address,bool)", u256(0x60), arg2, arg3, arg1.length(), arg1).empty());
    }

    // Call a contract function with a string and return a bytes32 value
    util::h256 callStringReturnsBytes32(const std::string& name, const std::string& arg)
    {
        const bytes& ret = call(name + "(string)", u256(0x20), arg.length(), arg);
        BOOST_REQUIRE(ret.size() == 0x20);
        return util::h256(ret);
    }

    // Call a contract function with a string and return an address
    util::h160 callStringReturnsAddress(const std::string& name, const std::string& arg)
    {
        const bytes& ret = call(name + "(string)", u256(0x20), arg.length(), arg);
        BOOST_REQUIRE(ret.size() == 0x20);
        BOOST_CHECK(std::count(ret.begin(), ret.begin() + 12, 0) == 12);
        bytes addr{ret.begin() + 12, ret.end()};
        return util::h160(addr);
    }

    // Call a contract function with an address and return a string
    std::string callAddressReturnsString(const std::string& name, const util::h160& arg)
    {
        const bytes& ret = call(name + "(address)", arg);
        BOOST_REQUIRE(ret.size() >= 0x40);
        u256 offset(util::h256(ret.cropped(0, 0x20)));
        BOOST_REQUIRE_EQUAL(offset, 0x20);
        u256 len(util::h256(ret.cropped(0x20, 0x20)));
        BOOST_REQUIRE_EQUAL(ret.size(), 0x40 + ((len + 0x1f) / 0x20) * 0x20);
        return ret.cropped(0x40, size_t(len)).toString();
    }

private:
    u256 m_nextValue;            // Next value for contract calls
    ExecutionFramework& m_framework; // Reference to the execution framework
};
} // namespace solidity::test
