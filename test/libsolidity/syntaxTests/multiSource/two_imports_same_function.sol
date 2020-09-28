==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
==== Source: s2.sol ====
import {f as f, f as f} from "s1.sol";
contract C {
  function g() public pure returns (uint) {
    return f();
  }
}
