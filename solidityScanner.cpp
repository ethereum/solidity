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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity scanner.
 */

#include <libsolidity/Scanner.h>
#include <boost/test/unit_test.hpp>

namespace dev {
namespace solidity {
namespace test {

BOOST_AUTO_TEST_SUITE(solidity)

BOOST_AUTO_TEST_CASE(smoke_test)
{
    Scanner scanner(CharStream("function break;765  \t  \"string1\",'string2'\nidentifier1"));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::FUNCTION);
    BOOST_CHECK_EQUAL(scanner.next(), Token::BREAK);
    BOOST_CHECK_EQUAL(scanner.next(), Token::SEMICOLON);
    BOOST_CHECK_EQUAL(scanner.next(), Token::NUMBER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "765");
    BOOST_CHECK_EQUAL(scanner.next(), Token::STRING);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "string1");
    BOOST_CHECK_EQUAL(scanner.next(), Token::COMMA);
    BOOST_CHECK_EQUAL(scanner.next(), Token::STRING);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "string2");
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "identifier1");
    BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(string_escapes)
{
    Scanner scanner(CharStream("  { \"a\\x61\""));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::LBRACE);
    BOOST_CHECK_EQUAL(scanner.next(), Token::STRING);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "aa");
}

BOOST_AUTO_TEST_CASE(string_escapes_with_zero)
{
    Scanner scanner(CharStream("  { \"a\\x61\\x00abc\""));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::LBRACE);
    BOOST_CHECK_EQUAL(scanner.next(), Token::STRING);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), std::string("aa\0abc", 6));
}

BOOST_AUTO_TEST_CASE(string_escape_illegal)
{
    Scanner scanner(CharStream(" bla \"\\x6rf\" (illegalescape)"));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "");
    // TODO recovery from illegal tokens should be improved
    BOOST_CHECK_EQUAL(scanner.next(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(hex_numbers)
{
    Scanner scanner(CharStream("var x = 0x765432536763762734623472346;"));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::VAR);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ASSIGN);
    BOOST_CHECK_EQUAL(scanner.next(), Token::NUMBER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLiteral(), "0x765432536763762734623472346");
    BOOST_CHECK_EQUAL(scanner.next(), Token::SEMICOLON);
    BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(locations)
{
    Scanner scanner(CharStream("function_identifier has ; -0x743/*comment*/\n ident //comment"));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 0);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 19);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 20);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 23);
    BOOST_CHECK_EQUAL(scanner.next(), Token::SEMICOLON);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 24);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 25);
    BOOST_CHECK_EQUAL(scanner.next(), Token::SUB);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 26);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 27);
    BOOST_CHECK_EQUAL(scanner.next(), Token::NUMBER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 27);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 32);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().beg_pos, 45);
    BOOST_CHECK_EQUAL(scanner.getCurrentLocation().end_pos, 50);
    BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(ambiguities)
{
    // test scanning of some operators which need look-ahead
    Scanner scanner(CharStream("<=""<""+ +=a++ =>""<<"));
    BOOST_CHECK_EQUAL(scanner.getCurrentToken(), Token::ILLEGAL);
    BOOST_CHECK_EQUAL(scanner.next(), Token::LTE);
    BOOST_CHECK_EQUAL(scanner.next(), Token::LT);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ADD);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ASSIGN_ADD);
    BOOST_CHECK_EQUAL(scanner.next(), Token::IDENTIFIER);
    BOOST_CHECK_EQUAL(scanner.next(), Token::INC);
    BOOST_CHECK_EQUAL(scanner.next(), Token::ARROW);
    BOOST_CHECK_EQUAL(scanner.next(), Token::SHL);
}


BOOST_AUTO_TEST_SUITE_END()

} } } // end namespaces
