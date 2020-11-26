pragma abicoder v2;

contract C {
    struct S {
        uint32 a;
        uint256[] b;
        uint64 c;
    }

    function f(S calldata s)
        external
        pure
        returns (uint32 a, uint256 b0, uint256 b1, uint64 c)
    {
        a = s.a;
        b0 = s.b[0];
        b1 = s.b[1];
        c = s.c;
    }
}
// ====
// compileViaYul: also
// ----
// f((uint32,uint256[],uint64)): 0x20, 42, 0x60, 23, 2, 1, 2 -> 42, 1, 2, 23
