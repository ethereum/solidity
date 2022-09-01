pragma abicoder v2;

contract C {
    struct S {
        uint128 p1;
        uint256[][2] a;
        uint32 p2;
    }

    function g(uint32 p1, S memory s) internal returns(uint32, uint128, uint256, uint256, uint32) {
        s.p1++;
        s.a[0][1]++;
        return (p1, s.p1, s.a[0][0], s.a[1][1], s.p2);
    }

    function f(uint32 p1, S calldata c) external returns(uint32, uint128, uint256, uint256, uint32) {
        return g(p1, c);
    }
}
// ----
// f(uint32,(uint128,uint256[][2],uint32)): 55, 0x40, 77, 0x60, 88, 0x40, 0x40, 2, 1, 2 -> 55, 78, 1, 2, 88
