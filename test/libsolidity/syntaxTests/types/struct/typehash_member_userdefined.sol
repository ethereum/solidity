type T is uint256;
contract C {
    struct S {
        T t;
    }

    bytes32 h = type(S).typehash;
}
