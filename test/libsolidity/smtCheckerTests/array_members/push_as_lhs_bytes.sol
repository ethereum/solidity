contract C {
	bytes b;

	function f() public {
		require(b.length == 0);
		b.push() = bytes1(uint8(1));
		assert(b[0] == bytes1(uint8(1)));
	}

	function g() public {
		bytes1 one = bytes1(uint8(1));
		b.push() = one;
		assert(b[b.length - 1] == one);
		// Fails
		assert(b[b.length - 1] == bytes1(uint8(100)));
	}

}
// ====
// SMTEngine: all
// ----
// Warning 6328: (265-310): CHC: Assertion violation happens here.\nCounterexample:\nb = [1]\none = 1\n\nTransaction trace:\nC.constructor()\nState: b = []\nC.g()
