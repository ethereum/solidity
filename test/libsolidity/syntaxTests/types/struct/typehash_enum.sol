contract C {
    enum E {
        VALUE
    }

    bytes32 h = type(E).typehash;
}
// ----
// TypeError 9582: (63-79): Member "typehash" not found or not visible after argument-dependent lookup in type(enum C.E).
