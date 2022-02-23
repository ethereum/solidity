contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "foo" hex"aa" unicode"😃, 😭, and 😈" "!" hex"00";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (106-113): Expected ';' but got 'HexStringLiteral'
