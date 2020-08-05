pragma experimental SMTChecker;
contract C {
	int[][][] array2d;
	function l() public {
		array2d.push().push().push();
		assert(array2d.length > 0);
		uint last = array2d[array2d.length - 1].length;
		assert(last > 0);
		assert(array2d[array2d.length - 1][last - 1].length > 0);
	}
}
