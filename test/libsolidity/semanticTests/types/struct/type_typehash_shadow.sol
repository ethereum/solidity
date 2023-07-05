struct S {
    uint256 x;
}

bytes32 constant TYPE_HASH_FILE_LEVEL = type(S).typehash;

contract C {
    struct S {
        uint256 y;
    }

    function f() public pure returns(bool, bool, bool) {
        return (
            type(S).typehash == keccak256("S(uint256 y)"),
            type(S).typehash == TYPE_HASH_FILE_LEVEL,
            TYPE_HASH_FILE_LEVEL == keccak256("S(uint256 x)")
        );
    }
}
// ----
// f() -> true, false, true
