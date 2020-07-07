contract C {
    uint immutable x = 0;
    uint y = x;
}
// ----
// TypeError 7733: (52-53): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
