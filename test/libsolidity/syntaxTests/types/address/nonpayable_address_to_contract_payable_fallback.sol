contract C {
  function f() public pure returns (C c) {
    address a = address(2);
    c = C(a);
  }
  function() external payable {
  }
}
// ----
// TypeError: (92-96): Explicit type conversion not allowed from non-payable "address" to "contract C", which has a payable fallback function.
