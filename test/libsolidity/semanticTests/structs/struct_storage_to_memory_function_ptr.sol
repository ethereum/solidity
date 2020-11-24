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

    uint[79] arr;
    X x = X(12, S(42, 23, 34, g));

    function f() external returns (uint32, uint128, uint256, uint32, uint32) {
        X memory m = x;
        return (m.s.a, m.s.b, m.s.c, m.s.f(), x.s.f());
    }

    function g() internal returns (uint32) {
        return x.s.a;
    }
}

// ====
// compileViaYul: false
// ----
// f() -> 42, 23, 34, 42, 42
