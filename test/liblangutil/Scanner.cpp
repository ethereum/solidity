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

#include <liblangutil/Scanner.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;

namespace solidity::langutil::test
{

BOOST_AUTO_TEST_SUITE(ScannerTest)

BOOST_AUTO_TEST_CASE(test_empty)
{
	CharStream stream{};
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CharStream stream("function break;765  \t  \"string1\",'string2'\nidentifier1", "");
	Scanner scanner(stream);
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

BOOST_AUTO_TEST_CASE(assembly_assign)
{
	CharStream stream("let a := 1", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Let);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssemblyAssign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "1");
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(assembly_multiple_assign)
{
	CharStream stream("let a, b, c := 1", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Let);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssemblyAssign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "1");
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(string_printable)
{
	for (unsigned v = 0x20; v < 0x7e; v++) {
		string lit{static_cast<char>(v)};
		// Escape \ and " (since we are quoting with ")
		if (v == '\\' || v == '"')
			lit = string{'\\'} + lit;
		CharStream stream("  { \"" + lit + "\"", "");
		Scanner scanner(stream);
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
		BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), string{static_cast<char>(v)});
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
	// Special case of unescaped " for strings quoted with '
	CharStream stream("  { '\"'", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "\"");
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(string_nonprintable)
{
	for (unsigned v = 0; v < 0xff; v++) {
		// Skip the valid ones
		if (v >= 0x20 && v <= 0x7e)
			continue;
		string lit{static_cast<char>(v)};
		CharStream stream("  { \"" + lit + "\"", "");
		Scanner scanner(stream);
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
		BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		if (v == '\n' || v == '\v' || v == '\f' || v == '\r')
			BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalStringEndQuote);
		else
			BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalCharacterInString);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
	}
}

BOOST_AUTO_TEST_CASE(string_escapes)
{
	CharStream stream("  { \"a\\x61\"", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "aa");
}

BOOST_AUTO_TEST_CASE(string_escapes_all)
{
	CharStream stream("  { \"a\\x61\\n\\r\\t\"", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "aa\n\r\t");
}

struct TestScanner
{
	unique_ptr<CharStream> stream;
	unique_ptr<Scanner> scanner;
	explicit TestScanner(string _text) { reset(move(_text)); }

	void reset(std::string _text)
	{
		stream = make_unique<CharStream>(move(_text), "");
		scanner = make_unique<Scanner>(*stream);
	}

	decltype(auto) currentToken() { return scanner->currentToken(); }
	decltype(auto) next() { return scanner->next(); }
	decltype(auto) currentError() { return scanner->currentError(); }
	decltype(auto) currentLiteral() { return scanner->currentLiteral(); }
	decltype(auto) currentCommentLiteral() { return scanner->currentCommentLiteral(); }
	decltype(auto) currentLocation() { return scanner->currentLocation(); }
};

BOOST_AUTO_TEST_CASE(string_escapes_legal_before_080)
{
	TestScanner scanner("  { \"a\\b");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalEscapeSequence);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
	scanner.reset("  { \"a\\f");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalEscapeSequence);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
	scanner.reset("  { \"a\\v");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalEscapeSequence);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(string_escapes_with_zero)
{
	TestScanner scanner("  { \"a\\x61\\x00abc\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("aa\0abc", 6));
}

BOOST_AUTO_TEST_CASE(string_escape_illegal)
{
	CharStream stream(" bla \"\\x6rf\" (illegalescape)", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalEscapeSequence);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "");
	// TODO recovery from illegal tokens should be improved
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(hex_numbers)
{
	TestScanner scanner("var x = 0x765432536763762734623472346;");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Assign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "0x765432536763762734623472346");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("0x1234");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "0x1234");
	scanner.reset("0X1234");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
}

BOOST_AUTO_TEST_CASE(octal_numbers)
{
	TestScanner scanner("07");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	scanner.reset("007");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	scanner.reset("-07");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	scanner.reset("-.07");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	scanner.reset("0");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	scanner.reset("0.1");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
}

BOOST_AUTO_TEST_CASE(scientific_notation)
{
	CharStream stream("var x = 2e10;", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Assign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "2e10");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Semicolon);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_dot_in_identifier)
{
	TestScanner scanner("function .a(");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("function .a(");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(middle_dot_in_identifier)
{
	TestScanner scanner("function a..a(");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("function a...a(");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(trailing_dot_in_identifier)
{
	TestScanner scanner("function a.(");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("function a.(");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(trailing_dot_in_numbers)
{
	TestScanner scanner("2.5");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("2.5e10");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset(".5");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset(".5e10");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("2.");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_underscore_decimal_is_identifier)
{
	// Actual error is cought by SyntaxChecker.
	CharStream stream("_1.2", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_underscore_decimal_after_dot_illegal)
{
	// Actual error is cought by SyntaxChecker.
	TestScanner scanner("1._2");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);

	scanner.reset("1._");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_underscore_exp_are_identifier)
{
	// Actual error is cought by SyntaxChecker.
	CharStream stream("_1e2", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_underscore_exp_after_e_illegal)
{
	// Actual error is cought by SyntaxChecker.
	CharStream stream("1e_2", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "1e_2");
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(leading_underscore_hex_illegal)
{
	CharStream stream("0x_abc", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(fixed_number_invalid_underscore_front)
{
	// Actual error is cought by SyntaxChecker.
	CharStream stream("12._1234_1234", "");
	Scanner scanner(stream);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(number_literals_with_trailing_underscore_at_eos)
{
	// Actual error is cought by SyntaxChecker.
	TestScanner scanner("0x123_");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);

	scanner.reset("123_");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);

	scanner.reset("12.34_");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(negative_numbers)
{
	TestScanner scanner("var x = -.2 + -0x78 + -7.3 + 8.9 + 2e-2;");
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
	TestScanner scanner("function_identifier has ; -0x743/*comment*/\n ident //comment");
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
	TestScanner scanner("<=" "<" "+ +=a++ =>" "<<" ">>" " >>=" ">>>" ">>>=" " >>>>>=><<=");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LessThanOrEqual);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LessThan);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Add);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignAdd);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Inc);
	BOOST_CHECK_EQUAL(scanner.next(), Token::DoubleArrow);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHL);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SAR);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignSar);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHR);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignShr);
	// the last "monster" token combination
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHR);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignSar);
	BOOST_CHECK_EQUAL(scanner.next(), Token::GreaterThan);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssignShl);
}

BOOST_AUTO_TEST_CASE(documentation_comments_parsed_begin)
{
	TestScanner scanner("/// Send $(value / 1000) chocolates to the user");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_comments_parsed_begin)
{
	TestScanner scanner("/** Send $(value / 1000) chocolates to the user*/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(documentation_comments_parsed)
{
	TestScanner scanner("some other tokens /// Send $(value / 1000) chocolates to the user");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_comments_parsed)
{
	TestScanner scanner("some other tokens /**\n"
						"* Send $(value / 1000) chocolates to the user\n"
						"*/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), " Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_no_stars)
{
	TestScanner scanner("some other tokens /**\n"
						" Send $(value / 1000) chocolates to the user\n"
						"*/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(multiline_documentation_whitespace_hell)
{
	TestScanner scanner("some other tokens /** \t \r \n"
						"\t \r  * Send $(value / 1000) chocolates to the user\n"
						"*/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), " Send $(value / 1000) chocolates to the user");
}

BOOST_AUTO_TEST_CASE(comment_before_eos)
{
	TestScanner scanner("//");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(documentation_comment_before_eos)
{
	TestScanner scanner("///");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(empty_multiline_comment)
{
	TestScanner scanner("/**/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(empty_multiline_documentation_comment_before_eos)
{
	TestScanner scanner("/***/");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::EOS);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
}

BOOST_AUTO_TEST_CASE(comments_mixed_in_sequence)
{
	TestScanner scanner("hello_world ///documentation comment \n"
						"//simple comment \n"
						"<<");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SHL);
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "documentation comment ");
}

BOOST_AUTO_TEST_CASE(ether_subdenominations)
{
	TestScanner scanner("wei gwei ether");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::SubWei);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubGwei);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubEther);
}

BOOST_AUTO_TEST_CASE(time_subdenominations)
{
	TestScanner scanner("seconds minutes hours days weeks years");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::SubSecond);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubMinute);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubHour);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubDay);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubWeek);
	BOOST_CHECK_EQUAL(scanner.next(), Token::SubYear);
}

BOOST_AUTO_TEST_CASE(empty_comment)
{
	TestScanner scanner("//\ncontract{}");
	BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Contract);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RBrace);

}

// Unicode string escapes

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape)
{
	TestScanner scanner("{ \"\\u00DAnicode\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xC3\x9Anicode", 8));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_7f)
{
	TestScanner scanner("{ \"\\u007Fnicode\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\x7Fnicode", 7));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_7ff)
{
	TestScanner scanner("{ \"\\u07FFnicode\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xDF\xBFnicode", 8));
}

BOOST_AUTO_TEST_CASE(valid_unicode_string_escape_ffff)
{
	TestScanner scanner("{ \"\\uFFFFnicode\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::StringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\xEF\xBF\xBFnicode", 9));
}

BOOST_AUTO_TEST_CASE(invalid_short_unicode_string_escape)
{
	TestScanner scanner("{ \"\\uFFnicode\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
}

// Unicode string literal

BOOST_AUTO_TEST_CASE(unicode_prefix_only)
{
	TestScanner scanner("{ unicode");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
	scanner.reset("{ unicode");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "unicode");
}

BOOST_AUTO_TEST_CASE(unicode_invalid_space)
{
	TestScanner scanner("{ unicode ");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
}

BOOST_AUTO_TEST_CASE(unicode_invalid_token)
{
	TestScanner scanner("{ unicode test");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
	scanner.reset("{ unicode test");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "unicode");
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), "test");
}

BOOST_AUTO_TEST_CASE(valid_unicode_literal)
{
	TestScanner scanner("{ unicode\"Hello 😃\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::UnicodeStringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("Hello \xf0\x9f\x98\x83", 10));
}

BOOST_AUTO_TEST_CASE(valid_nonprintable_in_unicode_literal)
{
	// Non-printable characters are allowed in unicode strings...
	TestScanner scanner("{ unicode\"Hello \007😃\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::UnicodeStringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("Hello \x07\xf0\x9f\x98\x83", 11));
}

// Hex string literal

BOOST_AUTO_TEST_CASE(hex_prefix_only)
{
	TestScanner scanner("{ hex");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
	scanner.reset("{ hex");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
}

BOOST_AUTO_TEST_CASE(hex_invalid_space)
{
	TestScanner scanner("{ hex ");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
}

BOOST_AUTO_TEST_CASE(hex_invalid_token)
{
	TestScanner scanner("{ hex test");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
	scanner.reset("{ hex test");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalToken);
}

BOOST_AUTO_TEST_CASE(valid_hex_literal)
{
	TestScanner scanner("{ hex\"00112233FF\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::HexStringLiteral);
	BOOST_CHECK_EQUAL(scanner.currentLiteral(), std::string("\x00\x11\x22\x33\xFF", 5));
}

BOOST_AUTO_TEST_CASE(invalid_short_hex_literal)
{
	TestScanner scanner("{ hex\"00112233F\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalHexString);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_with_space)
{
	TestScanner scanner("{ hex\"00112233FF \"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalHexString);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_with_wrong_quotes)
{
	TestScanner scanner("{ hex\"00112233FF'");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalHexString);
}

BOOST_AUTO_TEST_CASE(invalid_hex_literal_nonhex_string)
{
	TestScanner scanner("{ hex\"hello\"");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::LBrace);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.currentError(), ScannerError::IllegalHexString);
}

// Comments

BOOST_AUTO_TEST_CASE(invalid_multiline_comment_close)
{
	// This used to parse as "comment", "identifier"
	TestScanner scanner("/** / x");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(multiline_doc_comment_at_eos)
{
	// This used to parse as "whitespace"
	TestScanner scanner("/**");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(multiline_comment_at_eos)
{
	TestScanner scanner("/*");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(regular_line_break_in_single_line_comment)
{
	for (auto const& nl: {"\r", "\n", "\r\n"})
	{
		TestScanner scanner("// abc " + string(nl) + " def ");
		BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(irregular_line_breaks_in_single_line_comment)
{
	for (auto const& nl: {"\v", "\f", "\xE2\x80\xA8", "\xE2\x80\xA9"})
	{
		TestScanner scanner("// abc " + string(nl) + " def ");
		BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
		for (size_t i = 0; i < string(nl).size() - 1; i++)
			BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(regular_line_breaks_in_single_line_doc_comment)
{
	for (auto const& nl: {"\r", "\n", "\r\n"})
	{
		TestScanner scanner("/// abc " + string(nl) + " def ");
		BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "abc ");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(regular_line_breaks_in_multiline_doc_comment)
{
	// Test CR, LF, CRLF as line valid terminators for code comments.
	// Any accepted non-LF is being canonicalized to LF.
	for (auto const& nl : {"\r"s, "\n"s, "\r\n"s})
	{
		TestScanner scanner{"/// Hello" + nl + "/// World" + nl + "ident"};
		auto const& lit = scanner.currentCommentLiteral();
		BOOST_CHECK_EQUAL(lit, "Hello\n World");
		BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "Hello\n World");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "ident");
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(irregular_line_breaks_in_single_line_doc_comment)
{
	for (auto const& nl: {"\v", "\f", "\xE2\x80\xA8", "\xE2\x80\xA9"})
	{
		TestScanner scanner("/// abc " + string(nl) + " def ");
		BOOST_CHECK_EQUAL(scanner.currentCommentLiteral(), "abc ");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
		for (size_t i = 0; i < string(nl).size() - 1; i++)
			BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(regular_line_breaks_in_strings)
{
	for (auto const& nl: {"\r"s, "\n"s, "\r\n"s})
	{
		TestScanner scanner("\"abc " + nl + " def\"");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(irregular_line_breaks_in_strings)
{
	for (auto const& nl: {"\v", "\f", "\xE2\x80\xA8", "\xE2\x80\xA9"})
	{
		TestScanner scanner("\"abc " + string(nl) + " def\"");
		BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Illegal);
		for (size_t i = 0; i < string(nl).size(); i++)
			BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
		BOOST_CHECK_EQUAL(scanner.currentLiteral(), "def");
		BOOST_CHECK_EQUAL(scanner.next(), Token::Illegal);
		BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	}
}

BOOST_AUTO_TEST_CASE(solidity_keywords)
{
	// These are tokens which have a different meaning in Yul.
	string keywords = "return byte bool address var in true false leave switch case default";
	TestScanner scanner(keywords);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Return);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Byte);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Bool);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Address);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Var);
	BOOST_CHECK_EQUAL(scanner.next(), Token::In);
	BOOST_CHECK_EQUAL(scanner.next(), Token::TrueLiteral);
	BOOST_CHECK_EQUAL(scanner.next(), Token::FalseLiteral);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Switch);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Case);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Default);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset(keywords);
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::TrueLiteral);
	BOOST_CHECK_EQUAL(scanner.next(), Token::FalseLiteral);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Leave);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Switch);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Case);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Default);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(yul_keyword_like)
{
	TestScanner scanner("leave.function");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("leave.function");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(yul_identifier_with_dots)
{
	TestScanner scanner("mystorage.slot := 1");
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Period);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssemblyAssign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset("mystorage.slot := 1");
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::AssemblyAssign);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Number);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(yul_function)
{
	string sig = "function f(a, b) -> x, y";
	TestScanner scanner(sig);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RightArrow);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset(sig);
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RightArrow);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_CASE(yul_function_with_whitespace)
{
	string sig = "function f (a, b) - > x, y";
	TestScanner scanner(sig);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::GreaterThan);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
	scanner.reset(sig);
	scanner.scanner->setScannerMode(ScannerKind::Yul);
	BOOST_CHECK_EQUAL(scanner.currentToken(), Token::Function);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::LParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::RParen);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Sub);
	BOOST_CHECK_EQUAL(scanner.next(), Token::GreaterThan);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Comma);
	BOOST_CHECK_EQUAL(scanner.next(), Token::Identifier);
	BOOST_CHECK_EQUAL(scanner.next(), Token::EOS);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
