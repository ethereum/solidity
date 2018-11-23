contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "This a test
    }
}
// ----
// ParserError: (100-112): Expected string end-quote.
