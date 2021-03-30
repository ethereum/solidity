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
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6328: (155-178): CHC: Assertion violation happens here.
