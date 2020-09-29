==== Source: s1.sol ====
contract C {
  function f() public pure returns (uint) {
    return 1337;
  }
}
==== Source: s2.sol ====
import {C.f as g} from "s1.sol";
// ----
// ParserError 2314: (s2.sol:9-10): Expected '}' but got '.'
