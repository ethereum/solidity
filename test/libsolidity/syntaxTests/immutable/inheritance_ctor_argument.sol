contract B {
    uint immutable x;

    constructor(uint _x) public {
        x = _x;
    }
}

contract C is B {
    uint immutable y;
    constructor() B(y = 3) public { }
}
// ----
// TypeError: (155-156): Immutable variables can only be initialized inline or assigned directly in the constructor.
