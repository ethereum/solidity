// This caused a segfault in an earlier version
contract C {
    constructor() {}
}
contract D is C {
    constructor() C(5) {}
}
// ----
// TypeError 2973: (120-124): Wrong argument count for modifier invocation: 1 arguments given but expected 0.
