type T is uint256;
contract C {
    struct S {
        T t;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9518: (83-99): "typehash" cannot be used for structs with members of "T" type.
