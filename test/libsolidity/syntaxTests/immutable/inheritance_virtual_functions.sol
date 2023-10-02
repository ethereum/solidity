contract B {
    uint immutable x;

    constructor() {
        x = xInit();
    }

    function xInit() internal view virtual returns(uint) {
        return 3;
    }
}

contract C is B {
    function xInit() internal view override returns(uint) {
        return x;
    }
}
// ----
