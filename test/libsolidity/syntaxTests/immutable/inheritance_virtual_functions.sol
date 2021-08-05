contract B {
    uint immutable x;

    constructor() {
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
// TypeError 7733: (253-254): Immutable variables cannot be read before they are initialized.
