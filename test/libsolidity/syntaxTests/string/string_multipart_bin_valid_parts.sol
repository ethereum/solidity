contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = bin"10101010" bin"1011";
        return escapeCharacters;
    }
}
// ----
// ParserError 5428: (114-118): Expected numbers of bits multiple of 8.
