pragma experimental SMTChecker;

contract C
{
	uint x;
	function f(uint y) public returns (uint) {
		x = y;
		return x;
	}
	function g(uint y) public {
		require(y < 1000);
		uint z = this.f(y);
		// Fails as false positive because CHC does not support `this`.
		assert(z < 1000);
	}
}
// ----
// Warning 6328: (263-279): Assertion violation happens here
