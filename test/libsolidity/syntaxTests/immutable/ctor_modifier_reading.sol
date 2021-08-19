contract C {
    uint immutable x;
    constructor() readX {
        x = 3;
    }

    modifier readX() {
        _; f(x);
    }

    function f(uint a) internal pure {}
}
// ----
// TypeError 7733: (119-120): Immutable variables cannot be read before they are initialized.
