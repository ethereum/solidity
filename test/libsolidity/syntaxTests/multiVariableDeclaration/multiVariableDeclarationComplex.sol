contract D {
  struct S { uint a; uint b; }
}
contract C {
  function f() internal pure {
    (,,,D.S[10*2] storage x,) = g();
    x;
  }
  function g() internal pure returns (uint, uint, uint, D.S[20] storage x, uint) { x = x; }
}
// ----
// Warning 6321: (176-180): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (182-186): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (188-192): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (213-217): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
