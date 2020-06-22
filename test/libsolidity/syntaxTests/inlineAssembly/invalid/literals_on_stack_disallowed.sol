contract C {
    function f() pure public {
        assembly {
            1
        }
    }
}
// ----
// ParserError 6913: (85-86): Call or assignment expected.
