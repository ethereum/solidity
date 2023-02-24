function stringSuffix(string memory) pure suffix returns (string memory) {}
function bytesSuffix(bytes memory) pure suffix returns (bytes memory) {}

contract C {
    function testString() public pure {
        "" "" stringSuffix;
        '' '' stringSuffix;
        "" "" "" "" stringSuffix;
        ""''""'' "" stringSuffix;

        "abcd" "" stringSuffix;
        "" "efgh" stringSuffix;
        "abcd" "efgh" stringSuffix;
        "abcd""efgh" stringSuffix;
        "abcd" "efgh"stringSuffix;
        "abcd""efgh"stringSuffix;

        'abcd' '' stringSuffix;
        '' 'efgh' stringSuffix;
        'abcd' 'efgh' stringSuffix;
        'abcd''efgh' stringSuffix;
        'abcd' 'efgh'stringSuffix;
        'abcd''efgh'stringSuffix;

        "abcd" 'efgh' stringSuffix;
        'abcd' "efgh" stringSuffix;
    }

    function testHex() public pure {
        hex"" hex"" bytesSuffix;
        hex'' hex'' bytesSuffix;
        hex"" hex"" hex"" hex"" bytesSuffix;
        hex""hex''hex""hex'' hex"" bytesSuffix;

        hex"1122" hex"" bytesSuffix;
        hex"" hex"3344" bytesSuffix;
        hex"1122" hex"3344" bytesSuffix;
        hex"1122"hex"3344" bytesSuffix;
        hex"1122" hex"3344"bytesSuffix;
        hex"1122"hex"3344"bytesSuffix;

        hex'1122' hex'' bytesSuffix;
        hex'' hex'3344' bytesSuffix;
        hex'1122' hex'3344' bytesSuffix;
        hex'1122'hex'3344' bytesSuffix;
        hex'1122' hex'3344'bytesSuffix;
        hex'1122'hex'3344'bytesSuffix;

        hex"1122" hex'3344' bytesSuffix;
        hex'1122' hex"3344" bytesSuffix;
    }

    function testUnicode() public pure {
        unicode"" unicode"" stringSuffix;
        unicode'' unicode'' stringSuffix;
        unicode"" unicode"" unicode"" unicode"" stringSuffix;
        unicode""unicode''unicode""unicode'' unicode"" stringSuffix;

        unicode"😃" unicode"" stringSuffix;
        unicode"" unicode"😃" stringSuffix;
        unicode"😃" unicode"😃" stringSuffix;
        unicode"😃"unicode"😃" stringSuffix;
        unicode"😃" unicode"😃"stringSuffix;
        unicode"😃"unicode"😃"stringSuffix;

        unicode'😃' unicode'' stringSuffix;
        unicode'' unicode'😃' stringSuffix;
        unicode'😃' unicode'😃' stringSuffix;
        unicode'😃'unicode'😃' stringSuffix;
        unicode'😃' unicode'😃'stringSuffix;
        unicode'😃'unicode'😃'stringSuffix;

        unicode"😃" unicode'😃' stringSuffix;
        unicode'😃' unicode"😃" stringSuffix;
    }
}
