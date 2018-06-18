contract C {
    apply pure {
        function f() view {}
    }
}
// ----
// ParserError: (51-55): Cannot override modifier area's state mutability.
