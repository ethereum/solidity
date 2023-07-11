contract A {
	int[] a;
	constructor() {
		p();
	}
	function p() public {
		a.push(1);
	}
	function f() public {
		require(a.length == 1 && a[0] == 1);
		int[] storage u = a;
		assert(u[0] == 1); // should hold
		int[] memory b = new int[](2);
		a = b;
		// Access is safe but oob is reported due to aliasing.
		assert(u[0] == 1); // should fail
		A.a = b;
		// Access is safe but oob is reported due to aliasing.
		assert(u[0] == 1); // should fail
	}

	function push_v(int x) public {
		a.push(x);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (318-322): CHC: Out of bounds access happens here.
// Warning 6328: (311-328): CHC: Assertion violation happens here.
// Warning 6368: (422-426): CHC: Out of bounds access happens here.
// Warning 6328: (415-432): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
