contract test {
    function f() public {
        assembly {
            pop
        }
    }
}
// ----
// ParserError 2314: (85-86): Expected '(' but got '}'
