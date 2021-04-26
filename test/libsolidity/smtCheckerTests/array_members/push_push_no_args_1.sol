contract C {
	int[][] array2d;
	function l() public {
		array2d.push().push();
		assert(array2d.length > 0);
		assert(array2d[array2d.length - 1].length > 0);
	}
}
// ====
// SMTEngine: all
// ----
