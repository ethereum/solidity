contract C {
    uint immutable x;
    constructor() {
        while (true)
            x = 1;
    }
}
// ----
// TypeError 6672: (88-89): Cannot write to immutable here: Immutable variables cannot be initialized inside a loop.
