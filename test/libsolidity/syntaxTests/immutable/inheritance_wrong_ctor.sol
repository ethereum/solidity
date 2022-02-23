contract B {
    uint immutable x = 4;
}

contract C is B {
    constructor() {
        x = 3;
    }
}
// ----
// TypeError 7484: (88-89): Cannot write to immutable here: Immutable variables must be initialized in the constructor of the contract they are defined in.
