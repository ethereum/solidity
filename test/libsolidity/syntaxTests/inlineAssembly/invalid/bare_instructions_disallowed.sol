contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// ParserError 2314: (95-98): Expected '(' but got identifier
