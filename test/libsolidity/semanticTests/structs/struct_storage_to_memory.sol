pragma abicoder               v2;

contract C {
    struct S {
        uint32 a;
        uint128 b;
        uint256 c;
    }
    struct X {
        uint32 a;
        S s;
    }

    uint[79] arr;
    X x = X(12, S(42, 23, 34));

    function f() external returns (uint32, uint128, uint256) {
        X memory m = x;
        return (m.s.a, m.s.b, m.s.c);
    }
}
// ----
// f() -> 42, 23, 34
