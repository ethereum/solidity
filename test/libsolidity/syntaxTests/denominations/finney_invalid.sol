contract C {
    function f() public {
        uint x = 1 finney;
    }
}
// ----
// ParserError 2314: (58-64): Expected ';' but got identifier
