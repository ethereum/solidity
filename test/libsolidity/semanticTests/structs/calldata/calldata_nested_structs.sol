pragma abicoder v2;

contract C {
    struct S {
        uint128 p1;
        uint256[][2] a;
        uint32 p2;
    }

    struct S1 {
        uint128 u;
        S s;
    }

    struct S2 {
        S[2] array;
    }

    function f1(S1 calldata c) internal returns(S1 calldata) {
        return c;
    }

    function f(S1 calldata c, uint32 p) external returns(uint32, uint128, uint256, uint256, uint32) {
        S1 memory m = f1(c);
        assert(m.s.a[0][0] == c.s.a[0][0]);
        assert(m.s.a[1][1] == c.s.a[1][1]);
        return (p, m.s.p1, m.s.a[0][0], m.s.a[1][1], m.s.p2);
    }

    function g(S2 calldata c) external returns(uint128, uint256, uint256, uint32) {
        S2 memory m = c;
        assert(m.array[0].a[0][0] == c.array[0].a[0][0]);
        assert(m.array[0].a[1][1] == c.array[0].a[1][1]);
        return (m.array[1].p1, m.array[1].a[0][0], m.array[1].a[1][1], m.array[1].p2);
    }

    function h(S1 calldata c, uint32 p) external returns(uint32, uint128, uint256, uint256, uint32) {
        S memory m = c.s;
        assert(m.a[0][0] == c.s.a[0][0]);
        assert(m.a[1][1] == c.s.a[1][1]);
        return (p, m.p1, m.a[0][0], m.a[1][1], m.p2);
    }
}
// ====
// compileViaYul: also
// ----
// f((uint128,(uint128,uint256[][2],uint32)),uint32): 0x40, 44, 11, 0x40, 22, 0x60, 33, 0x40, 0x40, 2, 1, 2 -> 44, 22, 1, 2, 33
// g(((uint128,uint256[][2],uint32)[2])): 0x20, 0x20, 0x40, 0x40, 22, 0x60, 33, 0x40, 0x40, 2, 1, 2 -> 22, 1, 2, 33
// h((uint128,(uint128,uint256[][2],uint32)),uint32): 0x40, 44, 11, 0x40, 22, 0x60, 33, 0x40, 0x40, 2, 1, 2 -> 44, 22, 1, 2, 33
