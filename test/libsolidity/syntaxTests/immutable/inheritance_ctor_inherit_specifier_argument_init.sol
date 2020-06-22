contract B {
    uint immutable x;

    constructor(uint _x) public {
        x = _x;
    }
}

contract C is B(C.y = 3) {
    uint immutable y;
}
// ----
// TypeError 1581: (111-114): Immutable variables can only be initialized inline or assigned directly in the constructor.
