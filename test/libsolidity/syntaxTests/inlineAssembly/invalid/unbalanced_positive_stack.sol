contract test {
    function f() public {
        assembly {
            1
        }
    }
}
// ----
// ParserError 6913: (83-84): Call or assignment expected.
