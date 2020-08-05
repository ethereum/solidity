contract C {
    uint immutable x;
    constructor() {
        while (true)
            x = 1;
    }
}
// ----
// TypeError 6672: (88-89): Immutable variables can only be initialized once, not in a while statement.
