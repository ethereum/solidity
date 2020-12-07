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

#pragma once

#include <boost/test/unit_test.hpp>
#include <test/ExecutionFramework.h>

#include <functional>

namespace solidity::test
{

class ContractInterface
{
public:
	ContractInterface(ExecutionFramework& _framework): m_framework(_framework) {}

	void setNextValue(u256 const& _value) { m_nextValue = _value; }

protected:
	template <class... Args>
	bytes const& call(std::string const& _sig, Args const&... _arguments)
	{
		auto const& ret = m_framework.callContractFunctionWithValue(_sig, m_nextValue, _arguments...);
		m_nextValue = 0;
		return ret;
	}

	void callString(std::string const& _name, std::string const& _arg)
	{
		BOOST_CHECK(call(_name + "(string)", u256(0x20), _arg.length(), _arg).empty());
	}

	void callStringAddress(std::string const& _name, std::string const& _arg1, util::h160 const& _arg2)
	{
		BOOST_CHECK(call(_name + "(string,address)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
	}

	void callStringAddressBool(std::string const& _name, std::string const& _arg1, util::h160 const& _arg2, bool _arg3)
	{
		BOOST_CHECK(call(_name + "(string,address,bool)", u256(0x60), _arg2, _arg3, _arg1.length(), _arg1).empty());
	}

	void callStringBytes32(std::string const& _name, std::string const& _arg1, util::h256 const& _arg2)
	{
		BOOST_CHECK(call(_name + "(string,bytes32)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
	}

	util::h160 callStringReturnsAddress(std::string const& _name, std::string const& _arg)
	{
		bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
		BOOST_REQUIRE(ret.size() == 0x20);
		BOOST_CHECK(std::count(ret.begin(), ret.begin() + 12, 0) == 12);
		bytes const addr{ret.begin() + 12, ret.end()};
		return util::h160(addr);
	}

	std::string callAddressReturnsString(std::string const& _name, util::h160 const& _arg)
	{
		bytesConstRef const ret(&call(_name + "(address)", _arg));
		BOOST_REQUIRE(ret.size() >= 0x40);
		u256 offset(util::h256(ret.cropped(0, 0x20)));
		BOOST_REQUIRE_EQUAL(offset, 0x20);
		u256 len(util::h256(ret.cropped(0x20, 0x20)));
		BOOST_REQUIRE_EQUAL(ret.size(), 0x40 + ((len + 0x1f) / 0x20) * 0x20);
		return ret.cropped(0x40, size_t(len)).toString();
	}

	util::h256 callStringReturnsBytes32(std::string const& _name, std::string const& _arg)
	{
		bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
		BOOST_REQUIRE(ret.size() == 0x20);
		return util::h256(ret);
	}

private:
	u256 m_nextValue;
	ExecutionFramework& m_framework;
};

} // end namespaces

