contract B {
    uint immutable x = 4;
}

contract C is B {
    constructor() {
        x = 3;
    }
}
