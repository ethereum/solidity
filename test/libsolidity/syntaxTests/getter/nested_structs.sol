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
// TypeError 2763: (88-118): The following types are only supported for getters in ABIEncoderV2: struct C.Y memory. Either remove "public" or use "pragma experimental ABIEncoderV2;" to enable the feature.
