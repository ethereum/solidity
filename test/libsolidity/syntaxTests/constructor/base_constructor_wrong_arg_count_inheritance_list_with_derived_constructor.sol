contract C {
    constructor(uint, bool) {}
}

contract D is C(1, true, "a") { constructor() {} }
contract E is C(1) { constructor() {} }
// ----
// TypeError 7927: (61-76): Wrong argument count for constructor call: 3 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 7927: (112-116): Wrong argument count for constructor call: 1 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
