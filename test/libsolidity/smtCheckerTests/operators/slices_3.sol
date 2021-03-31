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
// SMTEngine: all
// ----
// Warning 6328: (226-257): CHC: Assertion violation might happen here.
// Warning 4661: (226-257): BMC: Assertion violation happens here.
