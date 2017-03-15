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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity scanner.
 */

#include <libsolidity/parsing/Scanner.h>
#include <boost/test/unit_test.hpp>

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SolidityScanner)

BOOST_AUTO_TEST_CASE(test_empty)
{
	Scanner scanner(CharStream(""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(smoke_test)
{
	Scanner scanner(CharStream("function break;765  \t  \"string1\",'string2'\nidentifier1"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Break);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "765");
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "string1");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "string2");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "identifier1");
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(string_escapes)
{
	Scanner scanner(CharStream("  { \"a\\x61\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "aa");
}

BOOST_AUTO_TEST_CASE(string_escapes_with_zero)
{
	Scanner scanner(CharStream("  { \"a\\x61\\x00abc\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("aa\0abc", 6));
}

BOOST_AUTO_TEST_CASE(string_escape_illegal)
{
	Scanner scanner(CharStream(" bla \"\\x6rf\" (illegalescape)"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
	// TODO recovery from illegal tokens should be improved
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(hex_numbers)
{
	Scanner scanner(CharStream("var x = 0x765432536763762734623472346;"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Assign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "0x765432536763762734623472346");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(octal_numbers)
{
	Scanner scanner(CharStream("07"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	scanner.reset(CharStream("007"), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	scanner.reset(CharStream("-07"), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	scanner.reset(CharStream("-.07"), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	scanner.reset(CharStream("0"), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	scanner.reset(CharStream("0.1"), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
}

BOOST_AUTO_TEST_CASE(scientific_notation)
{
	Scanner scanner(CharStream("var x = 2e10;"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Assign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "2e10");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(negative_numbers)
{
	Scanner scanner(CharStream("var x = -.2 + -0x78 + -7.3 + 8.9 + 2e-2;"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Assign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), ".2");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "0x78");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "7.3");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "8.9");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "2e-2");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(locations)
{
	Scanner scanner(CharStream("function_identifier has ; -0x743/*comment*/\n ident //comment"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLocation().start, 0);
	BOOST_CHECK_EQUAL(scanner.currentLocation().end, 19);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLocation().start, 20);
	BOOST_CHECK_EQUAL(scanner.currentLocation().end, 23);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.currentLocation().start, 24);
	BOOST_CHECK_EQUAL(scanner.currentLocation().end, 25);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLocation().start, 27);
	BOOST_CHECK_EQUAL(scanner.currentLocation().end, 32);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLocation().start, 45);
	BOOST_CHECK_EQUAL(scanner.currentLocation().end, 50);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(ambiguities)
{
	// test scanning of some operators which need look-ahead
	Scanner scanner(CharStream("<=""<""+ +=a++ =>""<<"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LessThanOrEqual);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LessThan);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignAdd);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Inc);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Arrow);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHL);
}

BOOST_AUTO_TEST_CASE(documentation_comments_parsed_begin)
{
	Scanner scanner(CharStream("/// Send $(value / 1000) chocolates to the user"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_comments_parsed_begin)
{
	Scanner scanner(CharStream("/** Send $(value / 1000) chocolates to the user*/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(documentation_comments_parsed)
{
	Scanner scanner(CharStream("some other tokens /// Send $(value / 1000) chocolates to the user"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_comments_parsed)
{
	Scanner scanner(CharStream("some other tokens /**\n"
							   "* Send $(value / 1000) chocolates to the user\n"
							   "*/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_no_stars)
{
	Scanner scanner(CharStream("some other tokens /**\n"
							   " Send $(value / 1000) chocolates to the user\n"
							   "*/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_whitespace_hell)
{
	Scanner scanner(CharStream("some other tokens /** \t \r \n"
							   "\t \r  * Send $(value / 1000) chocolates to the user\n"
							   "*/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(comment_before_eos)
{
	Scanner scanner(CharStream("//"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(documentation_comment_before_eos)
{
	Scanner scanner(CharStream("///"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(empty_multiline_comment)
{
	Scanner scanner(CharStream("/**/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(empty_multiline_documentation_comment_before_eos)
{
	Scanner scanner(CharStream("/***/"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(comments_mixed_in_sequence)
{
	Scanner scanner(CharStream("hello_world ///documentation comment \n"
							   "//simple comment \n"
							   "<<"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHL);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "documentation comment ");
}

BOOST_AUTO_TEST_CASE(ether_subdenominations)
{
	Scanner scanner(CharStream("wei szabo finney ether"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::SubWei);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubSzabo);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubFinney);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubEther);
}

BOOST_AUTO_TEST_CASE(time_subdenominations)
{
	Scanner scanner(CharStream("seconds minutes hours days weeks years"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::SubSecond);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubMinute);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubHour);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubDay);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubWeek);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubYear);
}

BOOST_AUTO_TEST_CASE(empty_comment)
{
	Scanner scanner(CharStream("//\ncontract{}"));
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Contract);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RBrace);

}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape)
{
	Scanner scanner(CharStream("{ \"\\u00DAnicode\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xC3\x9Anicode", 8));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_7f)
{
	Scanner scanner(CharStream("{ \"\\u007Fnicode\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\x7Fnicode", 7));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_7ff)
{
	Scanner scanner(CharStream("{ \"\\u07FFnicode\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xDF\xBFnicode", 8));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_ffff)
{
	Scanner scanner(CharStream("{ \"\\uFFFFnicode\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xEF\xBF\xBFnicode", 9));
}

BOOST_AUTO_TEST_CASE(invalid_short_unicode_string_escape)
{
	Scanner scanner(CharStream("{ \"\\uFFnicode\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}

BOOST_AUTO_TEST_CASE(valid_hex_literal)
{
	Scanner scanner(CharStream("{ hex\"00112233FF\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\x00\x11\x22\x33\xFF", 5));
}

BOOST_AUTO_TEST_CASE(invalid_short_hex_literal)
{
	Scanner scanner(CharStream("{ hex\"00112233F\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_with_space)
{
	Scanner scanner(CharStream("{ hex\"00112233FF \""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_with_wrong_quotes)
{
	Scanner scanner(CharStream("{ hex\"00112233FF'"));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_nonhex_string)
{
	Scanner scanner(CharStream("{ hex\"hello\""));
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
