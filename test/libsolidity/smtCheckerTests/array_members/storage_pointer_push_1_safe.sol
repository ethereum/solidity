contract C {
	int[][] array2d;
	function l() public {
		s();
		array2d[2].push();
		assert(array2d[2].length > 0);
	}
	function s() internal returns (int[] storage) {
		array2d.push();
		array2d.push();
		array2d.push();
		return array2d[2];
	}
}
// ====
// SMTEngine: all
// ----
