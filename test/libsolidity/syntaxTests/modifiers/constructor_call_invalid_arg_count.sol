// This caused a segfault in an earlier version
contract C {
    constructor() public {}
}
contract D is C {
    constructor() C(5) public {}
}
// ----
// TypeError: (127-131): Wrong argument count for modifier invocation: 1 arguments given but expected 0.
