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
/** @file Parser.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Parser.h"

#if _MSC_VER
#pragma warning(disable:4348)
#endif

#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_utree.hpp>

using namespace std;
using namespace dev;
using namespace dev::lll;
namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace sp = boost::spirit;

void dev::lll::killBigints(sp::utree const& _this)
{
	switch (_this.which())
	{
	case sp::utree_type::list_type: for (auto const& i: _this) killBigints(i); break;
	case sp::utree_type::any_type: delete _this.get<bigint*>(); break;
	default:;
	}
}

void dev::lll::debugOutAST(ostream& _out, sp::utree const& _this)
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
		case 6: _out << "$ "; debugOutAST(_out, _this.front()); break;
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

namespace dev {
namespace lll {
namespace parseTreeLLL_ {

template<unsigned N>
struct tagNode
{
	void operator()(sp::utree& n, qi::rule<string::const_iterator, qi::ascii::space_type, sp::utree()>::context_type& c) const
	{
		(boost::fusion::at_c<0>(c.attributes) = n).tag(N);
	}
};

}}}

void dev::lll::parseTreeLLL(string const& _s, sp::utree& o_out)
{
	using qi::standard::space;
	using qi::standard::space_type;
	using dev::lll::parseTreeLLL_::tagNode;
	using symbol_type = sp::basic_string<std::string, sp::utree_type::symbol_type>;
	using it = string::const_iterator;

	qi::rule<it, space_type, sp::utree()> element;
	qi::rule<it, string()> str = '"' > qi::lexeme[+(~qi::char_(std::string("\"") + '\0'))] > '"';
	qi::rule<it, string()> strsh = '\'' > qi::lexeme[+(~qi::char_(std::string(" ;$@()[]{}:\n\t") + '\0'))];
	qi::rule<it, symbol_type()> symbol = qi::lexeme[+(~qi::char_(std::string(" $@[]{}:();\"\x01-\x1f\x7f") + '\0'))];
	qi::rule<it, string()> intstr = qi::lexeme[ qi::no_case["0x"][qi::_val = "0x"] >> +qi::char_("0-9a-fA-F")[qi::_val += qi::_1]] | qi::lexeme[+qi::char_("0-9")[qi::_val += qi::_1]];
	qi::rule<it, sp::utree()> integer = intstr[qi::_val = px::construct<sp::any_ptr>(px::new_<bigint>(qi::_1))];
	qi::rule<it, space_type, sp::utree()> atom = integer[qi::_val = qi::_1] | (str | strsh)[qi::_val = qi::_1] | symbol[qi::_val = qi::_1];
	qi::rule<it, space_type, sp::utree::list_type()> seq = '{' > *element > '}';
	qi::rule<it, space_type, sp::utree::list_type()> mload = '@' > element;
	qi::rule<it, space_type, sp::utree::list_type()> sload = qi::lit("@@") > element;
	qi::rule<it, space_type, sp::utree::list_type()> mstore = '[' > element > ']' > -qi::lit(":") > element;
	qi::rule<it, space_type, sp::utree::list_type()> sstore = qi::lit("[[") > element > qi::lit("]]") > -qi::lit(":") > element;
	qi::rule<it, space_type, sp::utree::list_type()> calldataload = qi::lit("$") > element;
	qi::rule<it, space_type, sp::utree::list_type()> list = '(' > *element > ')';

	qi::rule<it, space_type, sp::utree()> extra = sload[tagNode<2>()] | mload[tagNode<1>()] | sstore[tagNode<4>()] | mstore[tagNode<3>()] | seq[tagNode<5>()] | calldataload[tagNode<6>()];
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
	auto ret = s.cbegin();
	try
	{
		qi::phrase_parse(ret, s.cend(), element, space, qi::skip_flag::dont_postskip, o_out);
	}
	catch (qi::expectation_failure<it> const& e)
	{
		std::string fragment(e.first, e.last);
		std::string loc = to_string(std::distance(s.cbegin(), e.first) - 1);
		std::string reason("Lexer failure at " + loc + ": '" + fragment + "'");
		BOOST_THROW_EXCEPTION(ParserException() << errinfo_comment(reason));
	}
	for (auto i = ret; i != s.cend(); ++i)
		if (!isspace(*i)) {
			BOOST_THROW_EXCEPTION(ParserException() << errinfo_comment("Non-whitespace left in parser"));
		}
}
