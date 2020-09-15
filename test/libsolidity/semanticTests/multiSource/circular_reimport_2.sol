==== Source: s1.sol ====
import {f as g, g as h} from "s2.sol";
function f() pure returns (uint) { return 1000 + h() - g(); }
==== Source: s2.sol ====
import {f as h} from "s1.sol";
function f() pure returns (uint) { return 2; }
function g() pure returns (uint) { return 4; }
==== Source: s3.sol ====
import "s2.sol";
contract C {
  function foo() public pure returns (uint) {
    return 10000 + f() - g() - h();
  }
}
// ====
// compileViaYul: also
// ----
// foo() -> 0x2324
