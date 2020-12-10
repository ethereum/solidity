pragma experimental SMTChecker;

contract C
{
	function f(address c) public view {
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
		assert(c == msg.sender);
	}
}
// ----
// Warning 6328: (155-178): CHC: Assertion violation happens here.\nCounterexample:\n\nc = 39\n\n\nTransaction trace:\nconstructor()\nf(39)
