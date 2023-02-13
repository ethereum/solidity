contract C {
    constructor(uint, bool) {}
}

contract D is C(1, true, "a") { constructor() C(1, true, "a") {} }
contract E is C(1) { constructor() C(1) {} }
// ----
// DeclarationError 3364: (93-108): Base constructor arguments given twice.
// DeclarationError 3364: (149-153): Base constructor arguments given twice.
// TypeError 7927: (61-76): Wrong argument count for constructor call: 3 arguments given but expected 2
// TypeError 2973: (93-108): Wrong argument count for modifier invocation: 3 arguments given but expected 2.
// TypeError 7927: (128-132): Wrong argument count for constructor call: 1 arguments given but expected 2
// TypeError 2973: (149-153): Wrong argument count for modifier invocation: 1 arguments given but expected 2.
