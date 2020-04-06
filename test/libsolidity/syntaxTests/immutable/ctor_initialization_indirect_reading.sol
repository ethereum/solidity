contract C {
    uint immutable x;
    constructor() public {
        x = f();
    }

    function f() public pure returns (uint) { return 3 + x; }
}
// ----
// TypeError: (143-144): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
