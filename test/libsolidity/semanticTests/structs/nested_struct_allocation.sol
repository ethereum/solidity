contract C {
  struct I {
    uint b;
    uint c;
  }
  struct S {
    I a;
  }

  function f() external returns (uint) {
    S memory s = S(I(1,2));
    return s.a.b;
  }
}
// ====
// requiresYulOptimizer: minimalStack
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1
