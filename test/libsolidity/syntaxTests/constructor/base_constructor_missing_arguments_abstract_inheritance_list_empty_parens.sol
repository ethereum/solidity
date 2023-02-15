abstract contract C {
    constructor(uint, bool) {}
}

abstract contract D is C() {}
abstract contract E is C() { constructor() {} }
abstract contract F is C() { constructor() C {} }
abstract contract G is C() { constructor() C() {} }
// ----
// DeclarationError 1563: (177-178): Modifier-style base constructor call without arguments.
// TypeError 7927: (79-82): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (109-112): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (157-160): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (177-178): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
// TypeError 7927: (207-210): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (227-230): Wrong argument count for modifier invocation: 0 arguments given but expected 2.
