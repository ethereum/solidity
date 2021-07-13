pragma abicoder               v2;

contract C {
    struct S {
        uint136 p;
        uint128[3] b;
        uint128[] c;
    }

    S[] s;

    function f() external returns (uint256, uint256, uint128, uint128) {
        S[] memory m = new S[](3);
        m[1] = S(0, [uint128(1), 2, 3], new uint128[](3));
        m[1].c[0] = 1;
        m[1].c[1] = 2;
        m[1].c[2] = 3;
        s = m;
        assert(s.length == m.length);
        assert(s[1].b[1] == m[1].b[1]);
        assert(s[1].c[0] == m[1].c[0]);
        return (s[1].b.length, s[1].c.length, s[1].b[2], s[1].c[0]);
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 3, 3, 3, 1
// gas irOptimized: 183445
