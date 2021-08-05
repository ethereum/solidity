pragma abicoder               v2;

contract C {
    struct S {
        uint128 p1;
        uint256[][2] a;
        uint32 p2;
    }
    S s;
    function f(uint32 p1, S calldata c) external returns(uint32, uint128, uint256, uint256, uint32) {
        s = c;
        assert(s.a[0][0] == c.a[0][0]);
        assert(s.a[1][1] == c.a[1][1]);
        return (p1, s.p1, s.a[0][0], s.a[1][1], s.p2);
    }
}
// ====
// compileViaYul: also
// ----
// f(uint32,(uint128,uint256[][2],uint32)): 55, 0x40, 77, 0x60, 88, 0x40, 0x40, 2, 1, 2 -> 55, 77, 1, 2, 88
// gas irOptimized: 203716
// gas legacy: 208984
// gas legacyOptimized: 203583
