pragma abicoder v2;


contract C {
    struct S {
        uint256 a;
        uint256[2] b;
        uint256 c;
    }

    function f(S calldata s)
        external
        pure
        returns (uint256 a, uint256 b0, uint256 b1, uint256 c)
    {
        a = s.a;
        b0 = s.b[0];
        b1 = s.b[1];
        c = s.c;
    }
}
// ----
// f((uint256,uint256[2],uint256)): 42, 1, 2, 23 -> 42, 1, 2, 23
