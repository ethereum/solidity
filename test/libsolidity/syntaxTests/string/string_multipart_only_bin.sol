contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = bin"10101010" bin"10111011" "11001100";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (128-138): Expected ';' but got 'StringLiteral'
