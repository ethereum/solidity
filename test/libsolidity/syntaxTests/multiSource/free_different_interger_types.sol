==== Source: s1.sol ====
function f(uint) pure returns (uint) { return 24; }
function g() pure returns (bool) { return true; }
==== Source: s2.sol ====
import {f as g, g as g} from "s1.sol";
contract C {
  function foo() public pure returns (uint, bool) {
    return (g(2), g());
  }
}
// ----
