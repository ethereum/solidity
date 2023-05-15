==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
contract C {
  function g() public pure virtual returns (uint) {
    return f();
  }
}
==== Source: s2.sol ====
import "s1.sol" as M;
function f() pure returns (uint) { return 6; }
contract D is M.C {
  function g() public pure override returns (uint) {
    return super.g() + f() * 10000;
  }
}
// ----
// g() -> 61337
