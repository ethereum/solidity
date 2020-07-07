contract B {
    uint immutable x;

    constructor() {
        x = 3;
    }
}

contract C is B {
    uint immutable y;
    constructor() {
        y = 3;
    }
}
// ----
