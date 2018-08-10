contract C {
    apply pure {
        apply payable {}
    }
}
// ----
// SyntaxError: (38-54): Cannot override parent modifier area's state mutability.
