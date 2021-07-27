contract A {
  function f() public pure returns (fixed x) {
    assembly { x := 1000 }
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1000
