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
 * Unit tests for UTF-8 validation.
 */

#include <libdevcore/CommonData.h>
#include <libdevcore/UTF8.h>

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(UTF8)

namespace {

bool isValidUTF8(string const& _value)
{
	size_t pos;
	return validateUTF8(asString(fromHex(_value)), pos);
}

bool isInvalidUTF8(string const& _value, size_t _expectedPos)
{
	size_t pos;
	if (validateUTF8(asString(fromHex(_value)), pos))
		return false;
	if (pos != _expectedPos)
		return false;
	return true;
}

}

BOOST_AUTO_TEST_CASE(valid)
{
	BOOST_CHECK(isValidUTF8("00"));
	BOOST_CHECK(isValidUTF8("20"));
	BOOST_CHECK(isValidUTF8("7f"));
	BOOST_CHECK(isValidUTF8("c281"));
	BOOST_CHECK(isValidUTF8("df81"));
	BOOST_CHECK(isValidUTF8("e0a081"));
	BOOST_CHECK(isValidUTF8("e18081"));
	BOOST_CHECK(isValidUTF8("ec8081"));
	BOOST_CHECK(isValidUTF8("ed8081"));
	BOOST_CHECK(isValidUTF8("ee8081"));
	BOOST_CHECK(isValidUTF8("ef8081"));
	BOOST_CHECK(isValidUTF8("f0908081"));
	BOOST_CHECK(isValidUTF8("f3808081"));
	BOOST_CHECK(isValidUTF8("f2808081"));
	BOOST_CHECK(isValidUTF8("f3808081"));
	BOOST_CHECK(isValidUTF8("f48e8081"));
}

BOOST_AUTO_TEST_CASE(invalid)
{
	// anything between 0x80 and 0xc0 is disallowed
	BOOST_CHECK(isInvalidUTF8("80", 0)); // invalid per table 3.6
	BOOST_CHECK(isInvalidUTF8("a0", 0)); // invalid per table 3.6
	BOOST_CHECK(isInvalidUTF8("c0", 0)); // invalid per table 3.7
	BOOST_CHECK(isInvalidUTF8("c1", 0)); // invalid per table 3.7
	BOOST_CHECK(isInvalidUTF8("c2", 0)); // too short (position is reported as the first byte)
	BOOST_CHECK(isInvalidUTF8("e08081", 2)); // e0 must be followed by >= a0
	BOOST_CHECK(isInvalidUTF8("e180", 0)); // too short
	BOOST_CHECK(isInvalidUTF8("ec80", 0)); // too short
	BOOST_CHECK(isInvalidUTF8("f08f8001", 2)); // f0 must be followed by >= 90
	BOOST_CHECK(isInvalidUTF8("f18080", 0)); // too short
	BOOST_CHECK(isInvalidUTF8("f4908081", 2)); // f4 must be followed by < 90
	// anything above 0xf7 is disallowed
	BOOST_CHECK(isInvalidUTF8("f8", 0)); // invalid per table 3.7
	BOOST_CHECK(isInvalidUTF8("f9", 0)); // invalid per table 3.7
}

BOOST_AUTO_TEST_CASE(corpus)
{
	string source = R"(
κόσμε

hélló

Ā ā Ă ă Ą ą

ƀ Ɓ Ƃ ƃ Ƅ ƅ

ɐ ɑ ɒ ɓ ɔ ɕ

ʰ ʱ ʲ ʳ ʴ ʵ

̀ ́ ̂ ̃ ̄ ̅

ϩ Ϫ ϫ Ϭ ϭ Ϯ

Ё Ђ Ѓ Є Ѕ І

Ա Բ Գ Դ Ե Զ

 ק ר ש ת װ ױ

ځ ڂ ڃ ڄ څ چ

ऑ ऒ ओ औ क ख

ও ঔ ক খ গ ঘ

ਘ ਙ ਚ ਛ ਜ ਝ

ઓ ઔ ક ખ ગ ઘ

ଗ ଘ ଙ ଚ ଛ ଜ

ஔ க ங ச ஜ ஞ

ఎ ఏ ఐ ఒ ఓ ఔ

ಓ ಔ ಕ ಖ ಗ ಘ

ഐ ഒ ഓ ഔ ക

ฒ ณ ด ต ถ ท

ມ ຢ ຣ ລ ວ ສ

༄ ༅ ༆ ༇ ༈ ༉

Ⴑ Ⴒ Ⴓ Ⴔ Ⴕ Ⴖ

ᄌ ᄍ ᄎ ᄏ ᄐ

Ḕ ḕ Ḗ ḗ Ḙ ḙ Ḛ

ἐ ἑ ἒ ἓ ἔ ἕ

₠ ₡ ₢ ₣ ₤ ₥

⃐ ⃑ ⃒ ⃓ ⃔ ⃕ ⃖ ⃗ ⃘ ⃙ ⃚

ℋ ℌ ℍ ℎ ℏ ℐ ℑ

⅓ ⅔ ⅕ ⅖ ⅗

∬ ∭ ∮ ∯ ∰

⌖ ⌗ ⌘ ⌙ ⌚ ⌛

␀ ␁ ␂ ␃ ␄ ␅

⑀ ⑁ ⑂ ⑃ ⑄

① ② ③ ④ ⑤

╘ ╙ ╚ ╛ ╜ ╝

▁ ▂ ▃ ▄ ▅ ▆

▤ ▥ ▦ ▧ ▨

♔ ♕ ♖ ♗ ♘ ♙

✈ ✉ ✌ ✍ ✎

ぁ あ ぃ い ぅ

ァ ア ィ イ ゥ

ㄅ ㄆ ㄇ ㄈ ㄉ

ㄱ ㄲ ㄳ ㄴ ㄵ

㆚ ㆛ ㆜ ㆝ ㆞

㈀ ㈁ ㈂ ㈃ ㈄

㌀ ㌁ ㌂ ㌃ ㌄

乺 乻 乼 乽 乾

걺 걻 걼 걽 걾

豈 更 車 賈 滑

שּׁ שּׂ אַ אָ אּ

ﮄ ﮅ ﮆ ﮇ ﮈ ﮉ

 ﺵ ﺶ ﺷ ﺸ

｢ ｣ ､ ･ ｦ ｧ ｨ ｩ
	)";
	size_t pos;
	BOOST_CHECK(validateUTF8(source, pos));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
