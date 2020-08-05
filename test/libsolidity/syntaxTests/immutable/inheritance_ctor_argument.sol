contract B {
    uint immutable x;

    constructor(uint _x) {
        x = _x;
    }
}

contract C is B {
    uint immutable y;
    constructor() B(y = 3) { }
}
// ----
// TypeError 1581: (148-149): Immutable variables can only be initialized inline or assigned directly in the constructor.
