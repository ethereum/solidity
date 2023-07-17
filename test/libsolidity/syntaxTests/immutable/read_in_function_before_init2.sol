contract C {
  uint immutable t = 2;
  uint x = f();
  function f() internal pure returns (uint) { return t; }
}
