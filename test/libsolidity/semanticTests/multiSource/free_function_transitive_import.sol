==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
contract C {
  function g() public pure returns (uint) {
    return f();
  }
}
==== Source: s2.sol ====
import "s1.sol";
contract D is C {
  function h() public pure returns (uint) {
    return g();
  }
}
==== Source: s3.sol ====
import "s2.sol";
import {f as f} from "s2.sol";
contract E is D {
  function i() public pure returns (uint) {
    return f();
  }
}

// ====
// compileToEwasm: also
// ----
// i() -> 1337
