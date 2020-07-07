contract C {
  mapping(int => int) a;
  function f() public {
    [a];
  }
}
// ----
// TypeError 1545: (66-69): Type mapping(int256 => int256) is only valid in storage.
