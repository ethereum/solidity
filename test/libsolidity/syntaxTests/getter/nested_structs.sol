contract C {
    struct Y {
        uint b;
    }
    struct X {
        Y a;
    }
    mapping(uint256 => X) public m;
}
// ----
// TypeError: (88-118): The following types are only supported for getters in the new experimental ABI encoder: struct C.Y memory. Either remove "public" or use "pragma experimental ABIEncoderV2;" to enable the feature.
