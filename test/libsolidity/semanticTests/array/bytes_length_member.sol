contract c {
    function set() public returns (bool) {
        data = msg.data;
        return true;
    }

    function getLength() public returns (uint256) {
        return data.length;
    }

    bytes data;
}
// ====
// compileViaYul: also
// ----
// getLength() -> 0
// set(): 1, 2 -> true
// gas irOptimized: 110570
// gas legacy: 110721
// gas legacyOptimized: 110562
// getLength() -> 68
