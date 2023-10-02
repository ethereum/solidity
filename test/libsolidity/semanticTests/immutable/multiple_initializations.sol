contract A {
    uint immutable x = x + 1;
    uint immutable y = x += 2;

    constructor(uint) m(x += 16) m(x += 32) {
        x += 64;
        x += 128;
    }

    modifier m(uint) {
        _;
    }

    function get() public returns (uint) {
        return x;
    }
}

contract B is A(A.x += 8) {
    constructor(uint) {}
}

contract C is B {
    constructor() B(x += 4) {}
}
// ----
// get() -> 0xff
