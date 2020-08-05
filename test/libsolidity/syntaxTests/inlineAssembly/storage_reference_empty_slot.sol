contract C {
    function f() public pure {
        assembly {
            let x := .slot
        }
    }
}
// ----
// ParserError 1856: (84-85): Literal or identifier expected.
