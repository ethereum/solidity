contract C {
    struct S {
        uint256[][10][] x;
    }

    bytes32 immutable h;

    constructor() {
        h = type(S).typehash;
    }
}