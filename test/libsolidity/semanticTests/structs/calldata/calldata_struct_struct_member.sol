pragma abicoder v2;

contract C {
    struct S {
        uint64 a;
        uint64 b;
    }
    struct S1 {
        uint256 a;
        S s;
        uint256 c;
    }

    function f(S1 calldata s1)
        external
        pure
        returns (uint256 a, uint64 b0, uint64 b1, uint256 c)
    {
        a = s1.a;
        b0 = s1.s.a;
        b1 = s1.s.b;
        c = s1.c;
    }
}
// ====
// compileViaYul: also
// ----
// f((uint256,(uint64,uint64),uint256)): 42, 1, 2, 23 -> 42, 1, 2, 23
