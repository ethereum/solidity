contract C {
    uint128[13] unused;
    uint32[] a;
    uint32[3] b;
    function f() public returns (uint32, uint256) {
        uint32[] memory m = new uint32[](3);
        m[0] = 1;
        m[1] = 2;
        m[2] = 3;
        a = m;
        assert(a[0] == m[0]);
        assert(a[1] == m[1]);
        assert(a[2] == m[2]);
        return (a[0], a.length);
    }
    function g() public returns (uint32, uint32, uint32) {
        uint32[3] memory m;
        m[0] = 1; m[1] = 2; m[2] = 3;
        a = m;
        b = m;
        assert(a[0] == b[0] && a[1] == b[1] && a[2] == b[2]);
        assert(a.length == b.length);
        return (a[0], b[1], a[2]);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 3
// g() -> 1, 2, 3
