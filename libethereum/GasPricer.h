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
/** @file GasPricer.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libethcore/Common.h>

namespace dev
{
namespace eth
{

class Block;
class BlockChain;

enum class TransactionPriority
{
	Lowest = 0,
	Low = 2,
	Medium = 4,
	High = 6,
	Highest = 8
};

static const u256 DefaultGasPrice = 20 * shannon;

class GasPricer
{
public:
	GasPricer() = default;
	virtual ~GasPricer() = default;

	virtual u256 ask(Block const&) const = 0;
	virtual u256 bid(TransactionPriority _p = TransactionPriority::Medium) const = 0;

	virtual void update(BlockChain const&) {}
};

class TrivialGasPricer: public GasPricer
{
public:
	TrivialGasPricer() = default;
	TrivialGasPricer(u256 const& _ask, u256 const& _bid): m_ask(_ask), m_bid(_bid) {}

	void setAsk(u256 const& _ask) { m_ask = _ask; }
	void setBid(u256 const& _bid) { m_bid = _bid; }

	u256 ask() const { return m_ask; }
	u256 ask(Block const&) const override { return m_ask; }
	u256 bid(TransactionPriority = TransactionPriority::Medium) const override { return m_bid; }

private:
	u256 m_ask = DefaultGasPrice;
	u256 m_bid = DefaultGasPrice;
};

}
}
