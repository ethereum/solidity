pragma experimental SMTChecker;

contract C {
	uint public x;

	function f() public view {
		uint y = this.x();
		assert(y == x); // This fails as false positive because of lack of support for external getters.
	}
}
// ----
// Warning 6328: (114-128): CHC: Assertion violation happens here.
