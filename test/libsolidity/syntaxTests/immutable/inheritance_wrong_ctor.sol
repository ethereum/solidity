contract B {
    uint immutable x = 4;
}

contract C is B {
    constructor() public {
        x = 3;
    }
}
// ----
// TypeError: (95-96): Immutable variables must be initialized in the constructor of the contract they are defined in.
// TypeError: (95-96): Immutable state variable already initialized.
