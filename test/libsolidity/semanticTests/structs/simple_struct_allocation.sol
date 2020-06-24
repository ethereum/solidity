contract C {
  struct S {
    uint a;
  }

  function f() external returns (uint) {
    S memory s = S(1);
    return s.a;
  }
}
// ====
// compileViaYul: also
// ----
// f() -> 1
