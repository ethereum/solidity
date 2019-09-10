contract test {
    function f() public {
        assembly {
            1
        }
    }
}
// ----
// ParserError: (83-84): Call or assignment expected.
