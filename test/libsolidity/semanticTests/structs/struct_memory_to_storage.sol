pragma abicoder               v2;

contract C {
    struct S {
        uint32 a;
        uint128 b;
        uint256 c;
    }

    struct X {
        uint256 a;
        S s;
    }

    uint[79] r;
    X x;

    function f() external returns (uint32, uint128, uint256) {
        X memory m = X(12, S(42, 23, 34));
        x = m;
        return (x.s.a, x.s.b, x.s.c);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 42, 23, 34
