contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = hex"aa" hex"b";
        return escapeCharacters;
    }
}
// ----
// ParserError 5428: (108-112): Expected even number of hex-nibbles.
