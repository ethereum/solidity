contract C {
    uint immutable x;
    constructor() {
        if (false)
            x = 1;
    }
}
// ----
// TypeError 4599: (86-87): Immutable variables must be initialized unconditionally, not in an if statement.
