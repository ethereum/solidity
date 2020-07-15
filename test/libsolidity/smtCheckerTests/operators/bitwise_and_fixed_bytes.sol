pragma experimental SMTChecker;
contract C {
  function f() public pure returns (byte) {
    return (byte("") & (""));
  }
}
// ----
// Warning 5084: (101-109): Type conversion is not yet fully supported and might yield false positives.
