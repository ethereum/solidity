contract C {
  function f() public view {
    address a = address(this);
    a;
  }
}
// ----
