contract B {
    uint immutable x = 3;

    function readX() internal view virtual returns(uint) {
        return x;
    }
}

contract C is B {
    constructor() public {
        super.readX();
    }

    function readX() internal view override returns(uint) {
        return 1;
    }
}
// ----
// TypeError: (114-115): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
