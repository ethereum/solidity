contract C {
    // Fool parser into parsing a constructor as a function type.
    constructor() public x;
}
// ----
// SyntaxError 2668: (83-106): Functions without implementation cannot have modifiers.
// DeclarationError 7576: (104-105): Undeclared identifier.
