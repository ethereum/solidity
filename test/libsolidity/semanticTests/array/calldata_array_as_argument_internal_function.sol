pragma abicoder v2;
contract Test {
    function f(uint256[] calldata c) internal returns (uint a, uint b) {
        return (c.length, c[0]);
    }

    function g(uint256[] calldata c) external returns (uint a, uint b) {
        return f(c);
    }

    function h(uint256[] calldata c, uint start, uint end) external returns (uint a, uint b) {
        return f(c[start: end]);
    }
}
// ====
// compileViaYul: also
// ----
// g(uint256[]): 0x20, 4, 1, 2, 3, 4 -> 4, 1
// h(uint256[], uint256, uint256): 0x60, 1, 3, 4, 1, 2, 3, 4 -> 2, 2
