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
/** @file rlp.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * RLP test functions.
 */

#include <fstream>
#include "../json_spirit/json_spirit_reader_template.h"
#include "../json_spirit/json_spirit_writer_template.h"
#include <RLP.h>
using namespace std;
using namespace eth;
namespace js = json_spirit;

namespace eth
{

template <> class UnitTest<2>
{
public:
	static void buildRLP(js::mValue& _v, RLPStream& _rlp)
	{
		if (_v.type() == js::array_type)
		{
			RLPStream s;
			for (auto& i: _v.get_array())
				buildRLP(i, s);
			_rlp.appendList(s.out());
		}
		else if (_v.type() == js::int_type)
			_rlp.append(_v.get_uint64());
		else if (_v.type() == js::str_type)
		{
			auto s = _v.get_str();
			if (s.size() && s[0] == '#')
				_rlp.append(bigint(s.substr(1)));
			else
				_rlp.append(s);
		}
	}

	int operator()()
	{
		js::mValue v;
		string s = asString(contents("../../tests/rlptest.json"));
		js::read_string(s, v);
		bool passed = true;
		for (auto& i: v.get_obj())
		{
			js::mObject& o = i.second.get_obj();
			cnote << i.first;
			RLPStream s;
			buildRLP(o["in"], s);
			if (!o["out"].is_null() && o["out"].get_str() != asHex(s.out()))
			{
				cwarn << "Test failed.";
				cwarn << "Test says:" << o["out"].get_str();
				cwarn << "Impl says:" << asHex(s.out());
				passed = false;
			}
		}
		return passed ? 0 : 1;
	}

};

}

int rlpTest()
{
	cnote << "Testing RLP...";
	return UnitTest<2>()();
}
