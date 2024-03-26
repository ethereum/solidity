contract C {
    struct S {
        S[] s;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9298: (66-82): "typehash" cannot be used for recursive structs.
