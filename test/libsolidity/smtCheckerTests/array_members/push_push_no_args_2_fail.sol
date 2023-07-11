contract C {
	int[][][] array2d;
	function l() public {
		array2d.push().push().push();
		assert(array2d.length > 2);
		uint last = array2d[array2d.length - 1].length;
		assert(last > 3);
		assert(array2d[array2d.length - 1][last - 1].length > 4);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (90-116): CHC: Assertion violation happens here.
// Warning 6328: (170-186): CHC: Assertion violation happens here.
// Warning 6328: (190-246): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
