pragma experimental SMTChecker;

contract C
{
	uint x;
	function f(uint y) public {
		x = y;
	}
	function g(uint y) public {
		require(y < 1000);
		this.f(y);
		// Fails as false positive because CHC does not support `this`.
		assert(x < 1000);
	}
}
// ----
// Warning 6328: (227-243): Assertion violation happens here
