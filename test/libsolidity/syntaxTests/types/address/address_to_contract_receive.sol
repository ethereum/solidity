contract C {
  function f() public pure returns (C c) {
    c = C(payable(2));
  }
  receive() external payable {
  }
}
// ----
