pragma abicoder               v2;


contract C {
    struct S1 {
        uint256 a;
        uint256 b;
    }
    struct S2 {
        uint256 a;
        uint256 b;
        S1 s;
        uint256 c;
    }

    function f(S2 calldata s)
        external
        pure
        returns (uint256 a, uint256 b, uint256 sa, uint256 sb, uint256 c)
    {
        return (s.a, s.b, s.s.a, s.s.b, s.c);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f((uint256,uint256,(uint256,uint256),uint256)): 1, 2, 3, 4, 5 -> 1, 2, 3, 4, 5
