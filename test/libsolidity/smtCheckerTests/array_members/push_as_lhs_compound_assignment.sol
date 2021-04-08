contract C {

	int[] u;

	function t() public {
		require(u.length == 0);
		u.push() -= 1;
		assert(u[0] < 0); // should hold
		assert(u[0] >= 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (128-145): CHC: Assertion violation happens here.\nCounterexample:\nu = [(- 1)]\n\nTransaction trace:\nC.constructor()\nState: u = []\nC.t()
