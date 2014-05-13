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
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include <boost/test/unit_test.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_utree.hpp>

int trieTest();
int rlpTest();
int daggerTest();
int cryptoTest();
int stateTest();
int vmTest();
int hexPrefixTest();
int peerTest(int argc, char** argv);

#include <libethcore/Log.h>
#include <libethereum/BlockInfo.h>
using namespace std;
using namespace eth;
namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace sp = boost::spirit;

template <typename Iterator> struct whitespace: qi::grammar<Iterator>
{
	qi::rule<Iterator> start;
	whitespace(): whitespace::base_type(start)
	{
		start = *boost::spirit::standard::space;// | (';' >> *(boost::spirit::standard::char_ - qi::eol) >> qi::eol);
	}
};

ostream& operator<<(ostream& _out, sp::utree const& _this)
{
	switch (_this.which())
	{
	case sp::utree_type::list_type: _out << "( "; for (auto const& i: _this) ::operator<<(_out, i) << " "; _out << ")"; break;
	case sp::utree_type::int_type: _out << _this.get<int>(); break;
	case sp::utree_type::string_type: _out << _this.get<string>(); break;
	default: _out << "nil";
	}
	return _out;
}

BOOST_AUTO_TEST_CASE(basic_tests)
{
	cnote << "Hello";
/*
	auto symbol = qi::lexeme[+(~qi::char_(std::string(" ();\"\x01-\x1f\x7f") + '\0'))];
	auto integer = qi::lexeme[ qi::no_case["#x"] >>  qi::hex] | qi::lexeme[ qi::no_case["#o"] >> qi::oct] | qi::lexeme[-qi::no_case["#d"] >> qi::int_];
	qi::rule<string::iterator, sp::utree()> atom = symbol | integer;
	qi::rule<string::iterator, sp::utree()> list;
	qi::rule<string::iterator, sp::utree()> element = atom | list;
	list = '(' > *element > ')';

	string input = "(suicide (caller))";
	sp::utree out;

	qi::parse(input.begin(), input.end(), element, out);
	cnote << out;
*/
/*	RLPStream s;
	BlockInfo::genesis().fillStream(s, false);
	std::cout << RLP(s.out()) << std::endl;
	std::cout << toHex(s.out()) << std::endl;
	std::cout << sha3(s.out()) << std::endl;*/

//	int r = 0;
//	r += daggerTest();
//	r += stateTest();
//	r += peerTest(argc, argv);
//	BOOST_REQUIRE(!r);
}

