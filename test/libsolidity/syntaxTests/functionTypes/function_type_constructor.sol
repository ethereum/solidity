contract C {
    // Fool parser into parsing a constructor as a function type.
    constructor() public x;
}
// ----
// Warning: (83-106): Modifiers of functions without implementation are ignored.
// DeclarationError: (104-105): Undeclared identifier.
