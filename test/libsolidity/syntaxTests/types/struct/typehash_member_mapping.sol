contract C {
    struct S {
        mapping (uint256 => uint256) x;
    }

    bytes32 h = type(S).typehash;
}
// ----
// TypeError 9518: (92-108): "typehash" cannot be used for structs with members of "mapping(uint256 => uint256)" type.
