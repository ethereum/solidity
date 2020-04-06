contract C {
    uint immutable x = 3;
    constructor() public {
        x++;
    }
}
// ----
// TypeError: (74-75): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
