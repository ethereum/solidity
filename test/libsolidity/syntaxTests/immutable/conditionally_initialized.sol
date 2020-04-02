contract C {
    uint immutable x;
    constructor() public {
        if (false)
            x = 1;
    }
}
// ----
// TypeError: (93-94): Immutable variables must be initialized unconditionally, not in an if statement.
