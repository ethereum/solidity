pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		(uint[3] memory a, uint[3] memory b) = ([uint(1), 2, 3], [uint(1), 2, 4]);
		assert(a[0] == b[0]);
		assert(a[1] == b[1]);
		assert(a[2] == b[2]); // fails
	}
}
// ----
// Warning 6328: (201-221): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
