==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
==== Source: s2.sol ====
import {f as g} from "s1.sol";
function f() pure returns (uint) { return 6; }
contract D {
  function h() public pure returns (uint) {
    return g() + f() * 10000;
  }
}
// ----
// h() -> 61337
