==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
contract C {}
==== Source: s2.sol ====
import "s1.sol";
function f() pure returns (uint) { return 42; }
contract D is C {}
// ----
// DeclarationError 1686: (s2.sol:17-64): Function with same name and parameter types defined twice.
