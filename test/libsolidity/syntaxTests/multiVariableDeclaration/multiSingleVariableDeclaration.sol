contract C {
  function f() internal returns (uint) {
    (uint a) = f();
    a;
  }
}
// ----
// Warning 5740: (78-79): Unreachable code.
