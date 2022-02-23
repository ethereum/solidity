contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// ParserError 6913: (95-98): Call or assignment expected.
