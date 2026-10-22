contract C {
    function testStringLiteralEquivalence(bytes calldata _input) public pure returns (bool) {
        return keccak256("abc") == keccak256(_input);
    }

    function testHexStringLiteralEquivalence(bytes calldata _input) public pure returns (bool) {
        return keccak256(hex"4d41") == keccak256(_input);
    }

    function testUnicodeStringLiteralEquivalence(bytes calldata _input) public pure returns (bool) {
        return keccak256(unicode"€") == keccak256(_input);
    }

    function testEmptyStringEquivalence(bytes calldata _input) public pure returns (bool) {
        return keccak256("") == keccak256(_input);
    }

    function testUnaryTupleEquivalence(bytes calldata _input) public pure returns (bool) {
        return keccak256(("abc")) == keccak256(_input);
    }
}
// ----
// testStringLiteralEquivalence(bytes): 0x20, 3, "abc" -> true
// testHexStringLiteralEquivalence(bytes): 0x20, 2, left(0x4d41) -> true
// testUnicodeStringLiteralEquivalence(bytes): 0x20, 3, left(0xe282ac) -> true
// testEmptyStringEquivalence(bytes): 0x20, 0 -> true
// testUnaryTupleEquivalence(bytes): 0x20, 3, "abc" -> true
