contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = hex"0000"
        "deaf"
        "feed";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (118-124): Expected ';' but got 'StringLiteral'
