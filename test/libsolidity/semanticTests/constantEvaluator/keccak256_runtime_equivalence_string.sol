uint constant STRING_LITERAL = uint(keccak256("1234abcd"));
uint constant HEX_LITERAL = uint(keccak256(hex"4d41"));
contract C {
    uint[STRING_LITERAL] array;
    uint[HEX_LITERAL] array2;

    function testStringLiteralRuntimeEquivalence() public view returns (bool) {
        uint runtime = uint(keccak256("1234abcd"));

        return array.length == runtime;
    }
    function testHexLiteralRuntimeEquivalence() public view returns (bool) {
        uint runtime = uint(keccak256(hex"4d41"));

        return array2.length == runtime;
    }
}
// ----
// testStringLiteralRuntimeEquivalence() -> true
// testHexLiteralRuntimeEquivalence() -> true

