contract C {
    // Fool parser into parsing a constructor as a function type.
    constructor() x;
}
// ----
// SyntaxError 2668: (83-99='constructor() x;'): Functions without implementation cannot have modifiers.
// DeclarationError 7920: (97-98='x'): Identifier not found or not unique.
