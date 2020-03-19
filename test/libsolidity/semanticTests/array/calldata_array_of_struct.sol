pragma experimental ABIEncoderV2;


contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(S[] calldata s)
        external
        pure
        returns (uint256 l, uint256 a, uint256 b, uint256 c, uint256 d)
    {
        l = s.length;
        a = s[0].a;
        b = s[0].b;
        c = s[1].a;
        d = s[1].b;
    }
}

// ----
// f((uint256,uint256)[]): 0x20, 0x2, 0x1, 0x2, 0x3, 0x4 -> 2, 1, 2, 3, 4
