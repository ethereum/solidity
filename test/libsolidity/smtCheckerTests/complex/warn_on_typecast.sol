pragma experimental SMTChecker;
contract C {
    function f() public pure returns (uint) {
        return uint8(1);
    }
}
// ----
// Warning: (106-114): Type conversion is not yet fully supported and might yield false positives.
