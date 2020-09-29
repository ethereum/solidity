==== Source: A.sol ====
contract A { function f() public {} }
==== Source: B.sol ====
import "A.sol";
pragma experimental SMTChecker;
contract C is A {}
