contract C {
    uint immutable x;
    constructor() {
        if (false)
            x = 1;
    }
}
// ----
// TypeError 4599: (86-87): Cannot write to immutable here: Immutable variables cannot be initialized inside an if statement.
