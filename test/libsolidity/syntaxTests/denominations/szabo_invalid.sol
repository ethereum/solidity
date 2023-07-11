contract C {
    function f() public {
        uint x = 1 szabo;
    }
}
// ----
// ParserError 2314: (58-63): Expected ';' but got identifier
