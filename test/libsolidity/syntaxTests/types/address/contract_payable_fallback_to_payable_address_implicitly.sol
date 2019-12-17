contract C {
  function f() public view {
    address payable a = this;
    a;
  }
  fallback() external payable {
  }
}
// ----
// Warning: (0-120): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
// TypeError: (46-70): Type contract C is not implicitly convertible to expected type address payable.
