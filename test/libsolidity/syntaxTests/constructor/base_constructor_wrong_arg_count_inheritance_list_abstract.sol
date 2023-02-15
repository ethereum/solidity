abstract contract C {
    constructor(uint, bool) {}
}

abstract contract D is C(1, true, "a") {}
abstract contract E is C(1) {}
// ----
// TypeError 7927: (79-94): Wrong argument count for constructor call: 3 arguments given but expected 2
// TypeError 7927: (121-125): Wrong argument count for constructor call: 1 arguments given but expected 2
