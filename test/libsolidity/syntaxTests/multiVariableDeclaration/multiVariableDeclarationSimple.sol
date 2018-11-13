contract C {
  function f() internal pure returns (uint, uint, uint, uint) {
    (uint a, uint b,,) = f();
    a; b;
  }
  function g() internal pure {
    (bytes memory a, string storage b) = h();
    a; b;
  }
  function h() internal pure returns (bytes memory, string storage s) { s = s; }
}
// ----
