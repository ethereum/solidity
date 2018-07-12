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
