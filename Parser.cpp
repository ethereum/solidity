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
/** @file Parser.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Parser.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <libethcore/CommonEth.h>

using namespace std;
using namespace eth;
namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace sp = boost::spirit;

void eth::killBigints(sp::utree const& _this)
{
	switch (_this.which())
	{
	case sp::utree_type::list_type: for (auto const& i: _this) killBigints(i); break;
	case sp::utree_type::any_type: delete _this.get<bigint*>(); break;
	default:;
	}
}

void eth::debugOutAST(ostream& _out, sp::utree const& _this)
{
	switch (_this.which())
	{
	case sp::utree_type::list_type:
		switch (_this.tag())
		{
		case 0: _out << "( "; for (auto const& i: _this) { debugOutAST(_out, i); _out << " "; } _out << ")"; break;
		case 1: _out << "@ "; debugOutAST(_out, _this.front()); break;
		case 2: _out << "@@ "; debugOutAST(_out, _this.front()); break;
		case 3: _out << "[ "; debugOutAST(_out, _this.front()); _out << " ] "; debugOutAST(_out, _this.back()); break;
		case 4: _out << "[[ "; debugOutAST(_out, _this.front()); _out << " ]] "; debugOutAST(_out, _this.back()); break;
		case 5: _out << "{ "; for (auto const& i: _this) { debugOutAST(_out, i); _out << " "; } _out << "}"; break;
		default:;
		}

		break;
	case sp::utree_type::int_type: _out << _this.get<int>(); break;
	case sp::utree_type::string_type: _out << "\"" << _this.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::string_type>>() << "\""; break;
	case sp::utree_type::symbol_type: _out << _this.get<sp::basic_string<boost::iterator_range<char const*>, sp::utree_type::symbol_type>>(); break;
	case sp::utree_type::any_type: _out << *_this.get<bigint*>(); break;
	default: _out << "nil";
	}
}

void eth::parseTreeLLL(string const& _s, sp::utree& o_out)
{
	using qi::ascii::space;
	typedef sp::basic_string<std::string, sp::utree_type::symbol_type> symbol_type;
	typedef string::const_iterator it;

	qi::rule<it, qi::ascii::space_type, sp::utree()> element;
	qi::rule<it, string()> str = '"' > qi::lexeme[+(~qi::char_(std::string("\"") + '\0'))] > '"';
	qi::rule<it, string()> strsh = '\'' > qi::lexeme[+(~qi::char_(std::string(" ;@()[]{}:") + '\0'))];
	qi::rule<it, symbol_type()> symbol = qi::lexeme[+(~qi::char_(std::string(" @[]{}:();\"\x01-\x1f\x7f") + '\0'))];
	qi::rule<it, string()> intstr = qi::lexeme[ qi::no_case["0x"][qi::_val = "0x"] >> *qi::char_("0-9a-fA-F")[qi::_val += qi::_1]] | qi::lexeme[+qi::char_("0-9")[qi::_val += qi::_1]];
	qi::rule<it, bigint()> integer = intstr;
	qi::rule<it, bigint()> multiplier = qi::lit("wei")[qi::_val = 1] | qi::lit("szabo")[qi::_val = szabo] | qi::lit("finney")[qi::_val = finney] | qi::lit("ether")[qi::_val = ether];
	qi::rule<it, qi::ascii::space_type, bigint()> quantity = integer[qi::_val = qi::_1] >> -multiplier[qi::_val *= qi::_1];
	qi::rule<it, qi::ascii::space_type, sp::utree()> atom = quantity[qi::_val = px::construct<sp::any_ptr>(px::new_<bigint>(qi::_1))] | (str | strsh)[qi::_val = qi::_1] | symbol[qi::_val = qi::_1];
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> seq = '{' > *element > '}';
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> mload = '@' > element;
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> sload = qi::lit("@@") > element;
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> mstore = '[' > element > ']' > -qi::lit(":") > element;
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> sstore = qi::lit("[[") > element > qi::lit("]]") > -qi::lit(":") > element;
	qi::rule<it, qi::ascii::space_type, sp::utree()> extra = sload[qi::_val = qi::_1, bind(&sp::utree::tag, qi::_val, 2)] | mload[qi::_val = qi::_1, bind(&sp::utree::tag, qi::_val, 1)] | sstore[qi::_val = qi::_1, bind(&sp::utree::tag, qi::_val, 4)] | mstore[qi::_val = qi::_1, bind(&sp::utree::tag, qi::_val, 3)] | seq[qi::_val = qi::_1, bind(&sp::utree::tag, qi::_val, 5)];
	qi::rule<it, qi::ascii::space_type, sp::utree::list_type()> list = '(' > *element > ')';
	element = atom | list | extra;

	string s;
	s.reserve(_s.size());
	bool incomment = false;
	bool instring = false;
	bool insstring = false;
	for (auto i: _s)
	{
		if (i == ';' && !instring && !insstring)
			incomment = true;
		else if (i == '\n')
			incomment = instring = insstring = false;
		else if (i == '"' && !insstring)
			instring = !instring;
		else if (i == '\'')
			insstring = true;
		else if (i == ' ')
			insstring = false;
		if (!incomment)
			s.push_back(i);
	}
	qi::phrase_parse(s.cbegin(), s.cend(), element, space, o_out);
}

