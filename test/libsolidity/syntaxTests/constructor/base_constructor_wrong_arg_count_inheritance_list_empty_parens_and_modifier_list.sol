contract C {
    constructor(uint, bool) {}
}

contract D is C() { constructor() C(1, true, "a") {} }
contract E is C() { constructor() C(1) {} }
// ----
// TypeError 7927: (61-64): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (81-96): Wrong argument count for modifier invocation: 3 arguments given but expected 2.
// TypeError 7927: (116-119): Wrong argument count for constructor call: 0 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (136-140): Wrong argument count for modifier invocation: 1 arguments given but expected 2.
