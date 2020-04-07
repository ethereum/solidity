contract C {
    uint immutable x = 0;
    uint y = f();

    function f() internal returns(uint) { return x; }
}
// ----
// TypeError: (107-108): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
