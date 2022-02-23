contract C {

	uint[] a;

	constructor () {
		a.push();
		a.push();
	}

	function check() public {
		require(a.length >= 2);
		require(a[1] == 0);
		conditional_store();
		assert(a[1] == 1); // should fail;
		assert(a[1] == 0); // should hold;
	}

	function conditional_store() internal {
		if (a[1] == 0) {
			return;
		}
		a[1] = 1;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (172-189): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\n\nTransaction trace:\nC.constructor()\nState: a = [0, 0]\nC.check()\n    C.conditional_store() -- internal call
