contract C {
    bytes32 x = type(C).typehash;
}
// ----
// TypeError 9582: (29-45): Member "typehash" not found or not visible after argument-dependent lookup in type(contract C).
