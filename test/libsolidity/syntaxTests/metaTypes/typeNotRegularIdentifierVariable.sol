contract Test {
    function f() public pure {
        uint type;
    }
}
// ----
// ParserError: (60-64): Expected ';' but got 'type'
