contract C {
    uint immutable x;
    constructor() {
        x = 3 + x;
    }
}
// ----
// TypeError 7733: (71-72): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
