contract C {
    uint immutable x;
    constructor() {
        delete x;
    }
}
// ----
// TypeError 3969: (70-71='x'): Immutable variables must be initialized using an assignment.
