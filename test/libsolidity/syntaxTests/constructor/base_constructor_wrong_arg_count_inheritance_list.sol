contract C {
    constructor(uint, bool) {}
}

contract D is C(1, true, "a") {}
contract E is C(1) {}
// ----
// TypeError 7927: (61-76): Wrong argument count for constructor call: 3 arguments given but expected 2
// TypeError 7927: (94-98): Wrong argument count for constructor call: 1 arguments given but expected 2
