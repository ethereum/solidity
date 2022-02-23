contract C {
  uint immutable t = 2;
  uint x = f();
  function f() internal pure returns (uint) { return t; }
}
// ----
// TypeError 7733: (106-107): Immutable variables cannot be read before they are initialized.
