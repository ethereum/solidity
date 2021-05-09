==== Source: A.sol ====
contract A { function f() public {} }
==== Source: B.sol ====
import "A.sol";
contract C is A {}
// ====
// SMTEngine: all
// ----
