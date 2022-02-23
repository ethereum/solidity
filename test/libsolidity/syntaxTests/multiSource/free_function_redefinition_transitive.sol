==== Source: s1.sol ====
function f() pure returns (uint) { return 1337; }
contract C {}
==== Source: s2.sol ====
import "s1.sol";
contract D is C {}
==== Source: s3.sol ====
import "s2.sol";
function f() pure returns (uint) { return 42; }
contract E is D {}
// ----
// DeclarationError 1686: (s3.sol:17-64): Function with same name and parameter types defined twice.
