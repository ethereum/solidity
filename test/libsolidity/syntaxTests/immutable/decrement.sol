contract C {
    uint immutable x;
    constructor() {
        x--;
    }
}
// ----
// TypeError 3969: (63-64='x'): Immutable variables must be initialized using an assignment.
