contract C {
  function f() public view {
    address payable a = address(this);
    a;
  }
  receive() external payable {
  }
}
// ----
