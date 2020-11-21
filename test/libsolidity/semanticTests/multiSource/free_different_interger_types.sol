==== Source: s1.sol ====
function f(uint24) pure returns (uint) { return 24; }
function g(bool) pure returns (bool) { return true; }
==== Source: s2.sol ====
import {f as g, g as g} from "s1.sol";
contract C {
  function foo() public pure returns (uint, bool) {
    return (g(2), g(false));
  }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// foo() -> 24, true
