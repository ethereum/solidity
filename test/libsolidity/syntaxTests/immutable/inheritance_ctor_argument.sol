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
