contract C {
    struct S {
        ufixed128x18 x;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9518: (75-91): "typehash" cannot be used for structs with members of "ufixed128x18" type.
