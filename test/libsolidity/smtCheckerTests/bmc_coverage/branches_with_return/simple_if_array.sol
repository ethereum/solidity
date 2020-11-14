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
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (205-222): BMC: Assertion violation happens here.
