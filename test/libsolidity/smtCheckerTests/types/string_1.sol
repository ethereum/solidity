pragma experimental SMTChecker;

contract C
{
	function f(string memory s1, string memory s2) public pure {
		assert(bytes(s1).length == bytes(s2).length);
	}
}
// ----
// Warning 6328: (110-154): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf(s1, s2)
