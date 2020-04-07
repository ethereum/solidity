contract C {
    uint immutable immutable x;
    uint immutable constant x;
}
// ----
// ParserError: (32-41): Mutability already set to "immutable"
// ParserError: (64-72): Mutability already set to "immutable"
