pragma experimental ABIEncoderV2;


contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(S calldata s) external pure returns (uint256, uint256) {
        S memory m = s;
        return (m.a, m.b);
    }
}

// ----
// f((uint256,uint256)): 42, 23 -> 42, 23
