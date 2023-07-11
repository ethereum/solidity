abstract contract C {
    constructor(uint, bool) {}
}

abstract contract D is C { constructor() C {} }
// ----
// DeclarationError 1563: (97-98): Modifier-style base constructor call without arguments.
// TypeError 2973: (97-98): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
