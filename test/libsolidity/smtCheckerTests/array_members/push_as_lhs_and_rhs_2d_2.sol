contract C {
	uint[][] b;
	function f() public {
		b.push().push() = b.push().push();
		uint length = b.length;
		assert(length >= 2);
		uint length1 = b[length - 1].length;
		uint length2 = b[length - 2].length;
		assert(length1 == 1);
		assert(length2 == 1);
		assert(b[length - 1][length1 - 1] == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 12 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
