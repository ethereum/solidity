contract C {
	int[][] array2d;
	function s() public returns (int[] memory) {
		array2d.push() = array2d.push();
		assert(array2d[array2d.length - 1].length == array2d[array2d.length - 2].length);
		return array2d[1];
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
