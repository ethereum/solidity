contract B {
    uint immutable x;

    constructor(uint _x) {
        x = _x;
    }
}

contract C is B(C.y) {
    uint immutable y;
    constructor() {
        y = 3;
    }
}
