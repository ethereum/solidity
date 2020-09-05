contract C {
    mapping (uint => uint[2][2**255]) A;
}
// ----
// TypeError 1534: (17-52): Type too large for storage.
