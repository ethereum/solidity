contract C {
    constructor(uint, bool) {}
}

contract D is C { constructor() C(1, true, "a") {} }
contract E is C { constructor() C(1) {} }
// ----
// TypeError 2973: (79-94): Wrong argument count for modifier invocation: 3 arguments given but expected 2.
// TypeError 2973: (132-136): Wrong argument count for modifier invocation: 1 arguments given but expected 2.
