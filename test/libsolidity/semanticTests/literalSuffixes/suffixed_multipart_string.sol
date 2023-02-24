function stringSuffix(string memory s) pure suffix returns (string memory) { return s; }
function bytesSuffix(bytes memory b) pure suffix returns (bytes memory) { return b; }

contract C {
    string public emptyString = "" '' "" "" stringSuffix;
    bytes public emptyHex = hex"" hex'' hex"" hex"" bytesSuffix;
    string public emptyUnicode = unicode"" unicode'' unicode"" unicode"" stringSuffix;

    string public string1 = "abcd" "" stringSuffix;
    string public string2 = "" "efgh" stringSuffix;
    string public string3 = "abcd" "efgh" stringSuffix;

    bytes public hex1 = hex"9798" hex"" bytesSuffix;
    bytes public hex2 = hex"" hex"99a0" bytesSuffix;
    bytes public hex3 = hex"9798" hex"99a0" bytesSuffix;

    string public unicode1 = unicode"😃" unicode"" stringSuffix;
    string public unicode2 = unicode"" unicode"😃" stringSuffix;
    string public unicode3 = unicode"😃" unicode"😃" stringSuffix;
}
// ----
// emptyString() -> 0x20, 0
// emptyHex() -> 0x20, 0
// emptyUnicode() -> 0x20, 0
// string1() -> 0x20, 4, "abcd"
// string2() -> 0x20, 4, "efgh"
// string3() -> 0x20, 8, "abcdefgh"
// hex1() -> 0x20, 2, 0x9798000000000000000000000000000000000000000000000000000000000000
// hex2() -> 0x20, 2, 0x99a0000000000000000000000000000000000000000000000000000000000000
// hex3() -> 0x20, 4, 0x979899a000000000000000000000000000000000000000000000000000000000
// unicode1() -> 0x20, 4, "\xf0\x9f\x98\x83"
// unicode2() -> 0x20, 4, "\xf0\x9f\x98\x83"
// unicode3() -> 0x20, 8, "\xf0\x9f\x98\x83\xf0\x9f\x98\x83"
