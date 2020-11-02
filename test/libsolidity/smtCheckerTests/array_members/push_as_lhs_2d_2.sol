pragma experimental SMTChecker;
contract C {
	int[][] array2d;
	function s() public returns (int[] memory) {
		array2d.push() = array2d.push();
		assert(array2d[array2d.length - 1].length == array2d[array2d.length - 2].length);
		return array2d[2];
	}
}
