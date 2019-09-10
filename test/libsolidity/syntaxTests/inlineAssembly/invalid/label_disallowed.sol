contract C {
    function f() pure public {
        assembly {
            label:
        }
    }
}
// ----
// ParserError: (80-81): Call or assignment expected.
