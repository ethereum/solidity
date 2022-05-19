pragma abicoder v2;

struct S {
    uint128 p1;
    uint256[][2] a;
    uint32 p2;
}
struct S1 {
    uint128 u;
    S s;
}

library L {
    function f(S1 memory m, uint32 p) external returns(uint32, uint128, uint256, uint256, uint32) {
        return (p, m.s.p1, m.s.a[0][0], m.s.a[1][1], m.s.p2);
    }
}

contract C {

    function f(S1 calldata c, uint32 p) external returns(uint32, uint128, uint256, uint256, uint32) {
        return L.f(c, p);
    }
}
// ----
// library: L
// f((uint128,(uint128,uint256[][2],uint32)),uint32): 0x40, 44, 11, 0x40, 22, 0x60, 33, 0x40, 0x40, 2, 1, 2 -> 44, 22, 1, 2, 33
