contract C {
	uint[] b;
	function f() public {
		b.push() = b.push();
		uint length = b.length;
		assert(length >= 2);
		assert(b[length - 1] == 0);
		assert(b[length - 1] == b[length - 2]);
		// Fails
		assert(b[length - 1] == 1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (204-230): CHC: Assertion violation happens here.\nCounterexample:\nb = [0, 0]\nlength = 2\n\nTransaction trace:\nC.constructor()\nState: b = []\nC.f()
