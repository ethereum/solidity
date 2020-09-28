function f() pure returns (uint) { return 1337; }
function f() view returns (uint) { return 42; }
contract C {
  function g() public pure virtual returns (uint) {
    return f();
  }
}
// ----
// DeclarationError 1686: (0-49): Function with same name and parameter types defined twice.
