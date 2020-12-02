pragma experimental SMTChecker;

contract C
{
	uint[] s;
	function f() public {
		uint[3] memory a = [uint(1), 2, 3];
		uint[4] memory b = [uint(1), 2, 4, 3];
		uint[4] memory c = b;
		assert(c.length == b.length);
		s = a;
		assert(s.length == a.length);

		assert(s.length == c.length); // fails
		assert(s[0] == c[0]);
		assert(s[1] == c[1]);
		assert(s[2] == c[2]); // fails
		assert(s[2] == c[3]);
	}
}
// ----
// Warning 6328: (259-287): CHC: Assertion violation happens here.\nCounterexample:\ns = [1, 2, 3]\n\n\n\nTransaction trace:\nconstructor()\nState: s = []\nf()
// Warning 6328: (348-368): CHC: Assertion violation happens here.\nCounterexample:\ns = [1, 2, 3]\n\n\n\nTransaction trace:\nconstructor()\nState: s = []\nf()
