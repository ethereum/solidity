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
 * Unit tests for the mini moustache class.
 */

#include <libdevcore/Whiskers.h>

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(WhiskersTest)

BOOST_AUTO_TEST_CASE(no_templates)
{
	string templ = "this text does not contain templates";
	BOOST_CHECK_EQUAL(Whiskers(templ).render(), templ);
}

BOOST_AUTO_TEST_CASE(basic_replacement)
{
	string templ = "a <b> x <c> -> <d>.";
	string result = Whiskers(templ)
		("b", "BE")
		("c", "CE")
		("d", "DE")
		.render();
	BOOST_CHECK_EQUAL(result, "a BE x CE -> DE.");
}

BOOST_AUTO_TEST_CASE(tag_unavailable)
{
	string templ = "<b>";
	Whiskers m(templ);
	BOOST_CHECK_THROW(m.render(), WhiskersError);
}

BOOST_AUTO_TEST_CASE(list_unavailable)
{
	string templ = "<#b></b>";
	Whiskers m(templ);
	BOOST_CHECK_THROW(m.render(), WhiskersError);
}

BOOST_AUTO_TEST_CASE(name_type_collision)
{
	string templ = "<b><#b></b>";
	Whiskers m(templ);
	m("b", "x");
	BOOST_CHECK_THROW(m("b", vector<map<string, string>>{}), WhiskersError);
}

BOOST_AUTO_TEST_CASE(conditional)
{
	string templ = "<?b>X</b>";
	BOOST_CHECK_EQUAL(Whiskers(templ)("b", true).render(), "X");
	BOOST_CHECK_EQUAL(Whiskers(templ)("b", false).render(), "");
}

BOOST_AUTO_TEST_CASE(conditional_with_else)
{
	string templ = "<?b>X<!b>Y</b>";
	BOOST_CHECK_EQUAL(Whiskers(templ)("b", true).render(), "X");
	BOOST_CHECK_EQUAL(Whiskers(templ)("b", false).render(), "Y");
}

BOOST_AUTO_TEST_CASE(conditional_plus_params)
{
	string templ = " - <?b>_<r><!b>^<t></b> - ";
	Whiskers m1(templ);
	m1("b", true);
	m1("r", "R");
	m1("t", "T");
	BOOST_CHECK_EQUAL(m1.render(), " - _R - ");

	Whiskers m2(templ);
	m2("b", false);
	m2("r", "R");
	m2("t", "T");
	BOOST_CHECK_EQUAL(m2.render(), " - ^T - ");
}

BOOST_AUTO_TEST_CASE(conditional_plus_list)
{
	string templ = " - <?b>_<#l><x></l><!b><#l><y></l></b> - ";
	Whiskers m(templ);
	m("b", false);
	vector<map<string, string>> list(2);
	list[0]["x"] = "1";
	list[0]["y"] = "a";
	list[1]["x"] = "2";
	list[1]["y"] = "b";
	m("l", list);
	BOOST_CHECK_EQUAL(m.render(), " - ab - ");
}

BOOST_AUTO_TEST_CASE(complicated_replacement)
{
	string templ = "a <b> x <complicated> \n <nested>>.";
	string result = Whiskers(templ)
		("b", "BE")
		("complicated", "CO<M>PL")
		("nested", "NEST")
		.render();
	BOOST_CHECK_EQUAL(result, "a BE x CO<M>PL \n NEST>.");
}

BOOST_AUTO_TEST_CASE(non_existing_list)
{
	string templ = "a <#b></b>";
	Whiskers m(templ);
	BOOST_CHECK_THROW(m.render(), WhiskersError);
}

BOOST_AUTO_TEST_CASE(empty_list)
{
	string templ = "a <#b></b>x";
	string result = Whiskers(templ)("b", vector<Whiskers::StringMap>{}).render();
	BOOST_CHECK_EQUAL(result, "a x");
}

BOOST_AUTO_TEST_CASE(list)
{
	string templ = "a<#b>( <g> - <h> )</b>x";
	vector<map<string, string>> list(2);
	list[0]["g"] = "GE";
	list[0]["h"] = "H";
	list[1]["g"] = "2GE";
	list[1]["h"] = "2H";
	string result = Whiskers(templ)("b", list).render();
	BOOST_CHECK_EQUAL(result, "a( GE - H )( 2GE - 2H )x");
}

BOOST_AUTO_TEST_CASE(recursive_list)
{
	// Check that templates resulting from lists are not expanded again
	string templ = "a<#b> 1<g>3 </b><x>";
	vector<map<string, string>> list(1);
	list[0]["g"] = "<x>";
	string result = Whiskers(templ)("x", "X")("b", list).render();
	BOOST_CHECK_EQUAL(result, "a 1<x>3 X");
}

BOOST_AUTO_TEST_CASE(list_can_access_upper)
{
	string templ = "<#b>(<a>)</b>";
	vector<map<string, string>> list(2);
	Whiskers m(templ);
	string result = m("a", "A")("b", list).render();
	BOOST_CHECK_EQUAL(result, "(A)(A)");
}

BOOST_AUTO_TEST_CASE(parameter_collision)
{
	string templ = "a <#b></b>";
	vector<map<string, string>> list(1);
	list[0]["a"] = "x";
	Whiskers m(templ);
	m("a", "X")("b", list);
	BOOST_CHECK_THROW(m.render(), WhiskersError);
}

BOOST_AUTO_TEST_CASE(invalid_param)
{
	string templ = "a <b >";
	Whiskers m(templ);
	BOOST_CHECK_THROW(m("b ", "X"), WhiskersError);
}

BOOST_AUTO_TEST_CASE(invalid_param_rendered)
{
	string templ = "a <b >";
	Whiskers m(templ);
	BOOST_CHECK_EQUAL(m.render(), templ);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
