pragma experimental SMTChecker;

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
// ----
// Warning 6328: (205-222): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\n\n\n\nTransaction trace:\nconstructor()\nState: a = [0, 0]\ncheck()
