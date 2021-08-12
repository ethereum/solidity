pragma abicoder               v2;

contract C {
    struct S {
        uint32 a;
        uint128 b;
        uint256 c;
        function() internal returns (uint32) f;
    }

    struct X {
        uint256 a;
        S s;
    }

    uint[79] r;
    X x;

    function f() external returns (uint32, uint128, uint256, uint32, uint32) {
        X memory m = X(12, S(42, 23, 34, g));
        x = m;
        return (x.s.a, x.s.b, x.s.c, x.s.f(), m.s.f());
    }

    function g() internal returns (uint32) {
        return x.s.a;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 42, 23, 34, 42, 42
// gas irOptimized: 110966
// gas legacy: 112021
// gas legacyOptimized: 110548
