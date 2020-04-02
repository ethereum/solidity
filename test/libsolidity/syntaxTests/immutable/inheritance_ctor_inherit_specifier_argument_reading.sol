contract B {
    uint immutable x;

    constructor(uint _x) public {
        x = _x;
    }
}

contract C is B(C.y) {
    uint immutable y;
    constructor() public {
        y = 3;
    }
}
// ----
// TypeError: (111-114): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
