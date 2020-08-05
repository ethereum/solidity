contract C {
    function f() pure public {
        assembly {
            label:
        }
    }
}
// ----
// ParserError 6913: (80-81): Call or assignment expected.
