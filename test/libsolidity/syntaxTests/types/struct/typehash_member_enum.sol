contract C {
    enum E {
        VALUE
    }

    struct S {
        E e;
    }

    bytes32 h = type(S).typehash;
}

// ----
// TypeError 9518: (98-114): "typehash" cannot be used for structs with members of "enum C.E" type.
