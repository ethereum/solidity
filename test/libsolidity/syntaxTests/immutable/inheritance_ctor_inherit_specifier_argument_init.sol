contract B {
    uint immutable x;

    constructor(uint _x) {
        x = _x;
    }
}

contract C is B(C.y = 3) {
    uint immutable y;
}
// ----
// TypeError 1581: (104-107): Immutable variables can only be initialized inline or assigned directly in the constructor.
