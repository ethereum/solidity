contract A {
    struct S {
        C.S[] s;
    }
}
contract C {
    struct S {
        A.S s;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9298: (119-135): "typehash" cannot be used for recursive structs.
