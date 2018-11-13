contract C {
  function f() public view {
    address payable a = address(this);
    a;
  }
}
// ----
// TypeError: (46-79): Type address is not implicitly convertible to expected type address payable.
