function f() pure returns (uint) { return 1337; }
function f() view returns (uint) { return 42; }
contract C {
  function g() public pure virtual returns (uint) {
    return f();
  }
}
// ----
// Warning 2018: (50-97): Function state mutability can be restricted to pure
