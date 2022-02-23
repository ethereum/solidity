contract C {
    uint immutable x;
    constructor() {
        x--;
    }
}
// ----
// TypeError 3969: (63-64): Immutable variables must be initialized using an assignment.
