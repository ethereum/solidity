// Test to see if cleanup is performed properly during array copying
contract C {
    uint128[] x;
    function f() public returns(bool) {
        x.push(42); x.push(42); x.push(42); x.push(42);
        uint128[] memory y = new uint128[](1);
        y[0] = 23;
        x = y;
        assembly { sstore(x.slot, 4) }

        assert(x[0] == 23);
        assert(x[1] == 0);

        assert(x[2] == 0);
        // Issue 9832: the cleanup was only performed for the first packed type leaving the rest of
        // the slot dirty.
        assert(x[3] == 0);

        return true;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
// gas irOptimized: 92737
// gas legacy: 93035
// gas legacyOptimized: 92257
