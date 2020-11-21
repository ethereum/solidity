==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
contract C {
  function g() public pure virtual returns (uint) {
    return f();
  }
}
==== Source: s2.sol ====
import "s1.sol";
contract D is C {
  function g() public pure virtual override returns (uint) {
    return super.g() + 1;
  }
}
==== Source: s3.sol ====
import "s2.sol";
contract E is D {
  function g() public pure override returns (uint) {
    return super.g() + 1;
  }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// g() -> 1339
