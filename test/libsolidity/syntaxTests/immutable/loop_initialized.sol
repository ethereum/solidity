contract C {
    uint immutable x;
    constructor() public {
        while (true)
            x = 1;
    }
}
// ----
// TypeError: (95-96): Immutable variables can only be initialized once, not in a while statement.
