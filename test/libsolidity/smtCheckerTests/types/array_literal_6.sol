pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint[3] memory a = [uint(1), 2, 3];
		uint[4] memory b = [uint(1), 2, 4, 3];
		uint[4] memory c = b;
		assert(a.length == c.length); // fails
		assert(a[0] == c[0]);
		assert(a[1] == c[1]);
		assert(a[2] == c[2]); // fails
		assert(a[2] == c[3]);
	}
}
// ----
// Warning 6328: (179-207): CHC: Assertion violation happens here.\nCounterexample:\n\na = [1, 2, 3]\nb = [1, 2, 4, 3]\nc = [1, 2, 4, 3]\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (268-288): CHC: Assertion violation happens here.\nCounterexample:\n\na = [1, 2, 3]\nb = [1, 2, 4, 3]\nc = [1, 2, 4, 3]\n\nTransaction trace:\nC.constructor()\nC.f()
