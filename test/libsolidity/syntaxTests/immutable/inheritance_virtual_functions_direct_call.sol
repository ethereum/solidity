contract B {
    uint immutable x = 3;

    function readX() internal virtual returns(uint) {
        return x;
    }
}

contract C is B {
    constructor() {
        B.readX;
    }

    function readX() internal override returns(uint) {
        return 3;
    }
}
// ----
// TypeError 7733: (109-110): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
