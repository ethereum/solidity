contract C {
  function f() internal returns (uint) {
    (uint a) = f();
    a;
  }
}
// ----
// Warning 6321: (46-50): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
