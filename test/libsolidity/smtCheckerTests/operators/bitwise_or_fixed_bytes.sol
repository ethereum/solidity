pragma experimental SMTChecker;
contract C {
  function f() public pure returns (byte) {
    return (byte(0x0F) | (byte(0xF0)));
  }
}
// ----
// Warning 5084: (101-111): Type conversion is not yet fully supported and might yield false positives.
// Warning 5084: (115-125): Type conversion is not yet fully supported and might yield false positives.
