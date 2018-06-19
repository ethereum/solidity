contract C {
  function f() public view {
    C c = address(2);
  }
}
// ----
// TypeError: (46-62): Type address is not implicitly convertible to expected type contract C.
