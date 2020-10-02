pragma experimental SMTChecker;
contract C {
  function f() public pure returns (byte) {
    return (~byte(0xFF));
  }
}
// ----
