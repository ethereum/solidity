contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "foo" "bar" hex"aa";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (112-119): Expected ';' but got 'HexStringLiteral'
