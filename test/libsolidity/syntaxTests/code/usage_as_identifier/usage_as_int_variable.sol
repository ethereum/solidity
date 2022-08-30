contract X {
    function codeAsString() external {
        uint32 calldata code;
    }
}
// ----
// ParserError 3548: (76-80): Location already specified.
// ParserError 2314: (80-81): Expected identifier but got ';'
