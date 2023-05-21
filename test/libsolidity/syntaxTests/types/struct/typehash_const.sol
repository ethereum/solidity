contract C {
    struct S {
        uint256[][10][] x;
    }

    bytes32 constant h = type(S).typehash;
}