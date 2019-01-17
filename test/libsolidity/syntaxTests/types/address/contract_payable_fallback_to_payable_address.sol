contract C {
  function f() public view {
    address payable a = address(this);
    a;
  }
  function() external payable {
  }
}
// ----
