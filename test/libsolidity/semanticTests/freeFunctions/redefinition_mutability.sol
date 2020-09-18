function f() pure returns (uint) { return 1337; }
function f() view returns (uint) { return 42; }
contract C {
  function g() public view returns (uint) {
    return f();
  }
}
// ====
// compileViaYul: also
// ----
// g() -> 1337
