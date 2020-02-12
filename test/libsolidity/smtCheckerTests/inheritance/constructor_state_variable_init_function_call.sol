pragma experimental SMTChecker;

contract C {
	uint x = f(2);
	constructor () public {
		assert(x == 2);
	}

	function f(uint y) internal view returns (uint) {
		assert(y > 0);
		assert(x == 0);
		return y;
	}
}
// ----
