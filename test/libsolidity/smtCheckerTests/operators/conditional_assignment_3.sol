pragma experimental SMTChecker;

contract C {
	function f(uint a, uint b) public pure {
		require(a < 10);
		require(b <= a);

		uint c = (b > 4) ? a++ : b++;
		assert(c > a);
	}
}
// ----
// Warning 6328: (161-174): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nb = 1\n\n\nTransaction trace:\nconstructor()\nf(0, 0)
