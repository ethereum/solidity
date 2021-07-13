contract C {
    uint256[] x;
    function f() public returns(uint256) {
        x.push(42); x.push(42); x.push(42); x.push(42);
        uint256[] memory y = new uint256[](1);
        y[0] = 23;
        x = y;
        assembly { sstore(x.slot, 4) }
        assert(x[1] == 0);
        assert(x[2] == 0);
        return x[3];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
// gas irOptimized: 135098
// gas legacy: 135313
// gas legacyOptimized: 134548
