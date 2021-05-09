==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
==== Source: s2.sol ====
import {f as g} from "s1.sol";
==== Source: s3.sol ====
import {g as h} from "s2.sol";
contract C {
  function foo() public pure returns (uint) {
    return h();
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// foo() -> 1337
