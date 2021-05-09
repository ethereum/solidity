contract C {
    function f(int[] calldata b, uint256 start, uint256 end) public returns (int) {
        int[] memory m = b[start:end];
        uint len = end - start;
        assert(len == m.length);
        for (uint i = 0; i < len; i++) {
            assert(b[start:end][i] == m[i]);
        }
        return [b[start:end]][0][0];
    }

    function g(int[] calldata b, uint256 start, uint256 end) public returns (int[] memory) {
        return b[start:end];
    }

    function h1(int[] memory b) internal returns (int[] memory) {
        return b;
    }

    function h(int[] calldata b, uint256 start, uint256 end) public returns (int[] memory) {
        return h1(b[start:end]);
    }
}
// ====
// compileViaYul: also
// ----
// f(int256[],uint256,uint256): 0x60, 1, 3, 4, 1, 2, 3, 4 -> 2
// g(int256[],uint256,uint256): 0x60, 1, 3, 4, 1, 2, 3, 4 -> 0x20, 2, 2, 3
// h(int256[],uint256,uint256): 0x60, 1, 3, 4, 1, 2, 3, 4 -> 0x20, 2, 2, 3
