contract B {
    uint immutable x;

    constructor() public {
        x = xInit();
    }

    function xInit() internal virtual returns(uint) {
        return 3;
    }
}

contract C is B {
    function xInit() internal override returns(uint) {
        return x;
    }
}
// ----
// TypeError: (260-261): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
