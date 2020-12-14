pragma experimental SMTChecker;
contract C {
	uint[] x;
	function f() public {
		require(x.length == 0);
		++x.push();
		assert(x.length == 1);
		assert(x[0] == 1); // should hold
		assert(x[0] == 42); // should fail
	}
}
// ----
// Warning 6328: (182-200): CHC: Assertion violation happens here.\nCounterexample:\nx = [1]\n\n\n\nTransaction trace:\nconstructor()\nState: x = []\nf()
