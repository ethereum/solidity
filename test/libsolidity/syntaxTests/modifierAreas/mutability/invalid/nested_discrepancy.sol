contract C {
    apply pure {
        apply payable {}
    }
}
// ----
// ParserError: (44-51): Cannot override parent modifier area's state mutability of "pure".
