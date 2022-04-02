contract Test {
    function f() public pure {
        uint type;
    }
}
// ----
// ParserError 2314: (60-64='type'): Expected ';' but got 'type'
