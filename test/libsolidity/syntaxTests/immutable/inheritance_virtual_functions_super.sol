contract B {
    uint immutable x = 3;

    function readX() internal view virtual returns(uint) {
        return x;
    }
}

contract C is B {
    constructor() {
        super.readX();
    }

    function readX() internal pure override returns(uint) {
        return 1;
    }
}
// ----
