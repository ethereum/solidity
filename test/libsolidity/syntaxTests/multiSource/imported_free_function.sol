==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
==== Source: s2.sol ====
import "s1.sol";
contract C {
  function g() public pure returns (uint) {
    return f();
  }
}
