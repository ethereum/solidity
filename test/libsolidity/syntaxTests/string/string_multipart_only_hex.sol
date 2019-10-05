contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = hex"aa" hex"bb" "cc";
        return escapeCharacters;
    }
}
// ----
// ParserError: (116-120): Expected ';' but got 'StringLiteral'
