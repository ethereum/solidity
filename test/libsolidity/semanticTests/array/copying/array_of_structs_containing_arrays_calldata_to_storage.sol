pragma abicoder               v2;

contract C {
    struct S {
        uint256[] a;
    }

    S[] s;

    function f(S[] calldata c) external returns (uint256, uint256) {
        s = c;
        assert(s.length == c.length);
        for (uint i = 0; i < s.length; i++) {
            assert(s[i].a.length == c[i].a.length);
            for (uint j = 0; j < s[i].a.length; j++) {
                assert(s[i].a[j] == c[i].a[j]);
            }
        }
        return (s[1].a.length, s[1].a[0]);
    }
}
// ====
// compileViaYul: true
// ----
// f((uint256[])[]): 0x20, 3, 0x60, 0x60, 0x60, 0x20, 3, 1, 2, 3 -> 3, 1
// gas irOptimized: 326771
