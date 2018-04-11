contract C {
    // Fool parser into parsing a constructor as a function type.
    constructor() x;
}
// ----
// Warning: (83-99): Modifiers of functions without implementation are ignored.
// DeclarationError: (97-98): Undeclared identifier.
