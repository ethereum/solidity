pragma abicoder v1;
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
// TypeError 2763: (108-138): The following types are only supported for getters in ABI coder v2: struct C.Y memory. Either remove "public" or use "pragma abicoder v2;" to enable the feature.
