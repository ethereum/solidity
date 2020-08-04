contract C {
    uint immutable x = 3;
    constructor() {
        delete x;
    }
}
// ----
// TypeError 1574: (74-75): Immutable state variable already initialized.
// TypeError 7733: (74-75): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
