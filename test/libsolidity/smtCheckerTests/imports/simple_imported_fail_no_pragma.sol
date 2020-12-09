==== Source: A.sol ====
contract A {
	function f(uint x) public pure {
		assert(x > 0);
	}
}
==== Source: B.sol ====
import "A.sol";
pragma experimental SMTChecker;
contract C is A {}
// ----
// Warning 6328: (A.sol:49-62): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
