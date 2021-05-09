pragma abicoder v2;

contract C {
    struct S {
        uint128 p1;
        uint256[3] a;
        uint32 p2;
    }
    function f(S[] calldata c) internal returns (S[] memory) {
        return c;
    }
    function g(S[] calldata c, uint256 s, uint256 e) public returns (S[] memory) {
        return f(c[s:e]);
    }

    function f1(uint256[3][] calldata c) internal returns (uint256[3][] memory) {
        return c;
    }
    function g1(uint256[3][] calldata c, uint256 s, uint256 e) public returns (uint256[3][] memory) {
        return f1(c[s:e]);
    }
}
// ====
// compileViaYul: also
// ----
// g((uint128,uint256[3],uint32)[],uint256,uint256): 0x60, 1, 3, 4, 55, 1, 2, 3, 66, 66, 2, 3, 4, 77, 77, 3, 4, 5, 88, 88, 4, 5, 6, 99 -> 0x20, 2, 66, 2, 3, 4, 77, 77, 3, 4, 5, 88
// g1(uint256[3][],uint256,uint256): 0x60, 1, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 -> 0x20, 2, 4, 5, 6, 7, 8, 9
