contract C {
    struct S {
        fixed128x18 x;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9518: (74-90): "typehash" cannot be used for structs with members of "fixed128x18" type.
