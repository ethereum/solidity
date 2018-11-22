contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "text \";
        return escapeCharacters;
    }
}
// ----
// ParserError: (100-109): Expected String end-quote.