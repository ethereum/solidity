contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "\t\b\f";
        return escapeCharacters;
    }
}
// ----
// ParserError 8936: (100-105): Invalid escape sequence.
