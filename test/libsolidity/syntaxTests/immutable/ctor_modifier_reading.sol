contract C {
    uint immutable x;
    constructor() readX public {
        x = 3;
    }

    modifier readX() {
        _; f(x);
    }

    function f(uint a) internal pure {}
}
// ----
// TypeError: (126-127): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
