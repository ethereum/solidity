contract C {
  mapping(int => int) a;
  function f() public {
    [a];
  }
}
// ----
// TypeError: (66-69): Type mapping(int256 => int256) is only valid in storage.
