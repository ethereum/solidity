pragma abicoder               v2;


contract C {
    struct S {
        uint256 a;
        uint256 b;
    }

    function f(S calldata s) external pure returns (uint256 a, uint256 b) {
        a = s.a;
        b = s.b;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f((uint256,uint256)): 42, 23 -> 42, 23
