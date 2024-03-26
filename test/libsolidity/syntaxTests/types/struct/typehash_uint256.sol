contract C {
    bytes32 h = type(uint256).typehash;
}
// ----
// TypeError 9582: (29-51): Member "typehash" not found or not visible after argument-dependent lookup in type(uint256).
