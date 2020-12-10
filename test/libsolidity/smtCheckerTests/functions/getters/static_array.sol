pragma experimental SMTChecker;


contract C {

	uint[2] public x = [42,1];

	function f() public view {
		assert(this.x(0) == x[0]); // should hold
		assert(this.x(1) == x[1]); // should hold
		assert(this.x(0) == 0); // should fail
	}
}
// ----
// Warning 6328: (195-217): CHC: Assertion violation happens here.\nCounterexample:\nx = [42, 1]\n\n\n\nTransaction trace:\nconstructor()\nState: x = [42, 1]\nf()
