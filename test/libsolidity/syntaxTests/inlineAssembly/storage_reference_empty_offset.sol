contract C {
    function f() public pure {
        assembly {
            let x := .offset
        }
    }
}
// ----
// ParserError 1856: (84-85): Literal or identifier expected.
