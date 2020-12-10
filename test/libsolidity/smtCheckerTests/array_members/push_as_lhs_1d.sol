pragma experimental SMTChecker;

contract C {
	uint[] b;

	function f() public {
		require(b.length == 0);
		b.push() = 1;
		assert(b[0] == 1);
	}

	function g() public {
		b.push() = 1;
		assert(b[b.length - 1] == 1);
		// Fails
		assert(b[b.length - 1] == 100);
	}

}
// ----
// Warning 6328: (232-262): CHC: Assertion violation happens here.\nCounterexample:\nb = [1]\n\n\n\nTransaction trace:\nconstructor()\nState: b = []\ng()
