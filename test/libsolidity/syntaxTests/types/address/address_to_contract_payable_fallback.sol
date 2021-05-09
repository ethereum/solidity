contract C {
  function f() public pure returns (C c) {
    c = C(payable(address(2)));
  }
  fallback() external payable {
  }
}
// ----
// Warning 3628: (0-129): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
