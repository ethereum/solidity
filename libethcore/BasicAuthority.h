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
/** @file BasicAuthority.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Determines the PoW algorithm.
 */

#pragma once

#include "SealEngine.h"

namespace dev
{
namespace eth
{

class BasicAuthority: public SealEngineBase
{
public:
	std::string name() const override { return "BasicAuthority"; }
	unsigned revision() const override { return 0; }
	unsigned sealFields() const override { return 1; }
	bytes sealRLP() const override { return rlp(Signature()); }

	void populateFromParent(BlockHeader&, BlockHeader const&) const override;
	StringHashMap jsInfo(BlockHeader const& _bi) const override;
	void verify(Strictness _s, BlockHeader const& _bi, BlockHeader const& _parent, bytesConstRef _block) const override;
	bool shouldSeal(Interface*) override;
	void generateSeal(BlockHeader const& _bi) override;

	static Signature sig(BlockHeader const& _bi) { return _bi.seal<Signature>(); }
	static BlockHeader& setSig(BlockHeader& _bi, Signature const& _sig) { _bi.setSeal(_sig); return _bi; }
	void setSecret(Secret const& _s) { m_secret = _s; }
	static void init();

private:
	bool onOptionChanging(std::string const& _name, bytes const& _value) override;

	Secret m_secret;
	AddressHash m_authorities;
};

}
}
