contract test {
    function f() public {
        assembly {
            pop
        }
    }
}
// ----
// ParserError: (85-86): Expected '(' but got '}'
