contract C {
    uint immutable x;
    constructor() {
        delete x;
    }
}
// ----
// TypeError 3969: (70-71): Immutable variables must be initialized using an assignment.
