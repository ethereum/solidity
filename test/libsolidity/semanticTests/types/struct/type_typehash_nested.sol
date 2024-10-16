contract C {
    struct S2 {
        uint256 x;
        S1 y;
    }

    struct S1 {
        uint256 z;
        S3[] w;
    }

    struct S3 {
        address a;
        uint24 b;
    }

    function f() public pure returns(bool, bool, bool) {
        return (
            type(S3).typehash == keccak256("S3(address a,uint24 b)"),
            type(S1).typehash == keccak256("S1(uint256 z,S3[] w)S3(address a,uint24 b)"),
            type(S2).typehash == keccak256("S2(uint256 x,S1 y)S1(uint256 z,S3[] w)S3(address a,uint24 b)")
        );
    }
}
// ----
// f() -> true, true, true
