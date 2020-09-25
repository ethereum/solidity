==== Source:  ====
import "A.sol";
pragma experimental SMTChecker;
contract C is A {}
==== Source: A.sol ====
pragma experimental SMTChecker;
contract A {
	function f(uint x) public pure {
		assert(x > 0);
	}
}
// ----
// Warning 6328: (A.sol:81-94): CHC: Assertion violation happens here.
// Warning 6328: (A.sol:81-94): CHC: Assertion violation happens here.
