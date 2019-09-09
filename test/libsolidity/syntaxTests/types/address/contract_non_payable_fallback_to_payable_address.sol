contract C {
  function f() public view {
    address payable a = address(this);
    a;
  }
  fallback() external {
  }
}
// ----
// TypeError: (46-79): Type address is not implicitly convertible to expected type address payable.
