contract C {
	int[][] array2d;
	function s() public {
		delete array2d.push();
		assert(array2d[array2d.length - 1].length == 0);
		// Fails
		assert(array2d[array2d.length - 1].length != 0);
		delete array2d.push().push();
		uint length = array2d.length;
		uint length2 = array2d[length - 1].length;
		assert(array2d[length - 1][length2 - 1] == 0);
		// Fails
		assert(array2d[length - 1][length2 - 1] != 0);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (143-190): CHC: Assertion violation happens here.
// Warning 6328: (363-408): CHC: Assertion violation happens here.
