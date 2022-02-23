contract C {
    function f() public pure {
        (uint a, uint b, uint c);
    }
}
// ----
// ParserError 2314: (76-77): Expected '=' but got ';'
