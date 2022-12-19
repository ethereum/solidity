library L
{
	function add(uint x, uint y) internal pure returns (uint) {
		require(x < 1000);
		require(y < 1000);
		return x + y;
	}
}

contract C
{
	using L for uint;
	function f(uint x) public pure {
		uint y = x.add(999);
		assert(y < 1000);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (228-244): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\ny = 1000\n\nTransaction trace:\nC.constructor()\nC.f(1)\n    L.add(1, 999) -- internal call
