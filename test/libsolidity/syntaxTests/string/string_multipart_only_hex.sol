contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = hex"aa" hex"bb" "cc";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (116-120='"cc"'): Expected ';' but got 'StringLiteral'
