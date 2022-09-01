function f() pure returns (uint) { return 1337; }
contract C {
  function f() public pure returns (uint) {
    return f();
  }
}
// ====
// compileToEwasm: also
// ----
// f() -> FAILURE
