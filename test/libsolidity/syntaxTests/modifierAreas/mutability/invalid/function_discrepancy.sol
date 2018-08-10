contract C {
    apply pure {
        function f() public view {}
    }
}
// ----
// SyntaxError: (38-65): Cannot override modifier area's state mutability.
