==== Source: A.sol ====
contract A { function f() public {} }
==== Source:====
import "A.sol";
pragma experimental SMTChecker;
contract C is A {}
