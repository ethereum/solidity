contract C {
int[] s;
function f(int[] calldata b, uint256 start, uint256 end) public returns (int) {
    s = b[start:end];
    uint len = end - start;
    assert(len == s.length);
    for (uint i = 0; i < len; i++) {
        // Removed because of Spacer nondeterminism.
        //assert(b[start:end][i] == s[i]);
    }
    return s[0];
}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
