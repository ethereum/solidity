contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = bin"0000000000000000"
        "1101111010101111"
        "1111111011101101"
        "10101011";
        return escapeCharacters;
    }
}
// ----
// ParserError 2314: (130-148): Expected ';' but got 'StringLiteral'
