contract C {
    // Fool parser into parsing a constructor as a function type.
    constructor() x;
}
// ----
// SyntaxError 2668: (83-99): Functions without implementation cannot have modifiers.
// DeclarationError 7576: (97-98): Undeclared identifier.
