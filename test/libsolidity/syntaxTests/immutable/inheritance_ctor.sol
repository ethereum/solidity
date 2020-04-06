contract B {
    uint immutable x;

    constructor() public {
        x = 3;
    }
}

contract C is B {
    uint immutable y;
    constructor() public {
        y = 3;
    }
}
// ----
