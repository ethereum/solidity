contract X {
    function codeAsInt() external {
        string calldata code;
    }
}
// ----
// ParserError 3548: (73-77): Location already specified.
// ParserError 2314: (77-78): Expected identifier but got ';'
