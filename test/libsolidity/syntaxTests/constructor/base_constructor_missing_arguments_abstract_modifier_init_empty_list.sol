abstract contract C {
    constructor(uint, bool) {}
}

abstract contract D is C { constructor() C() {} }
// ----
// TypeError 2973: (97-100): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
