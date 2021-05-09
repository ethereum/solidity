function f(uint) returns (uint) {
    return 2;
}
function f(string memory) returns (uint) {
    return 3;
}

contract C {
  function g() public returns (uint, uint) {
      return (f(2), f("abc"));
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// g() -> 2, 3
