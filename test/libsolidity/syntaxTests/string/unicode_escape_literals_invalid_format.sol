contract test {
    // this function returns an invalid unicode character
    function invalidLiteral() public pure returns (bytes32) {
        bytes32 invalid = "\u00xx";
        return invalid;
    }

}
// ----
// ParserError 8936: (162-165): Invalid escape sequence.
