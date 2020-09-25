pragma experimental SMTChecker;
contract C {
  function f() public pure returns (byte) {
    return (~byte(0xFF));
  }
}
// ----
// Warning 5084: (102-112): Type conversion is not yet fully supported and might yield false positives.
