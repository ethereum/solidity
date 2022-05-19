pragma abicoder v2;

contract C {
    struct S {
        uint128 a;
        uint64 b;
        uint128 c;
    }
    function f(S[3] calldata c) public returns (uint128, uint64, uint128) {
        S[3] memory m = c;
        return (m[2].a, m[1].b, m[0].c);
    }
    function g(S[] calldata c) public returns (uint128, uint64, uint128) {
        S[] memory m = c;
        return (m[2].a, m[1].b, m[0].c);
    }
}
// ----
// f((uint128,uint64,uint128)[3]): 0, 0, 12, 0, 11, 0, 10, 0, 0 -> 10, 11, 12
// g((uint128,uint64,uint128)[]): 0x20, 3, 0, 0, 12, 0, 11, 0, 10, 0, 0 -> 10, 11, 12
