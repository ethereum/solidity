==== Source: s1.sol ====
import {f as g, g as h} from "s2.sol";
function f() pure returns (uint) { return 1000 + g() - h(); }
==== Source: s2.sol ====
import {f as h} from "s1.sol";
function f() pure returns (uint) { return 2; }
function g() pure returns (uint) { return 4; }
contract C {
  function foo() public pure returns (uint) {
    return h() - f() - g();
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// foo() -> 992
