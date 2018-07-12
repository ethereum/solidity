contract C {
    function f() public pure {
        (uint a, uint b, uint c);
    }
}
// ----
// ParserError: (76-77): Expected '=' but got ';'
