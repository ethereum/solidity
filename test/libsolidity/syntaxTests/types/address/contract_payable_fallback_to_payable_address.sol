contract C {
  function f() public view {
    address payable a = payable(this);
    a;
  }
  fallback() external payable {
  }
}
// ----
// Warning 3628: (0-129): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
