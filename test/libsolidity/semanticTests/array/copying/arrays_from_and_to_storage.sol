contract Test {
    uint24[] public data;
    function set(uint24[] memory _data) public returns (uint) {
        data = _data;
        return data.length;
    }
    function get() public returns (uint24[] memory) {
        return data;
    }
}
// ====
// compileViaYul: also
// ----
// set(uint24[]): 0x20, 18, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 -> 18
// gas irOptimized: 100010
// gas legacy: 103815
// gas legacyOptimized: 101382
// data(uint256): 7 -> 8
// data(uint256): 15 -> 16
// data(uint256): 18 -> FAILURE
// get() -> 0x20, 18, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
