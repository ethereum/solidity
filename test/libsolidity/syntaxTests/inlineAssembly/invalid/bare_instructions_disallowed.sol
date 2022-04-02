contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// ParserError 6913: (95-98='pop'): Call or assignment expected.
