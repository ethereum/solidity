contract C {
  function f() public pure returns (C c) {
    c = C(address(2));
  }
  receive() external payable {
  }
}
// ----
