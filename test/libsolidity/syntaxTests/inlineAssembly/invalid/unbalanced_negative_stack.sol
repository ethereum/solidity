contract test {
    function f() public {
        assembly {
            pop
        }
    }
}
// ----
// ParserError 6913: (85-86): Call or assignment expected.
