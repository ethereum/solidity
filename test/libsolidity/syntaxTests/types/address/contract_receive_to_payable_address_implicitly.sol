contract C {
  function f() public view {
    address payable a = this;
    a;
  }
  receive() external payable {
  }
}
// ----
// TypeError 9574: (46-70): Type contract C is not implicitly convertible to expected type address payable.
