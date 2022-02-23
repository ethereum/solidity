contract C {
    int[] s;
    function f(int[] calldata b, uint256 start, uint256 end) public returns (int) {
        s = b[start:end];
        uint len = end - start;
        assert(len == s.length);
        for (uint i = 0; i < len; i++) {
            assert(b[start:end][i] == s[i]);
        }
        return s[0];
    }
}
// ====
// compileViaYul: also
// ----
// f(int256[],uint256,uint256): 0x60, 1, 3, 4, 1, 2, 3, 4 -> 2
