contract C {
  function f() public pure returns (C c) {
    address a = address(2);
    c = C(a);
  }
  fallback() external payable {
  }
}
// ----
// Warning: (0-139): This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.
// TypeError: (92-96): Explicit type conversion not allowed from non-payable "address" to "contract C", which has a payable fallback function.
