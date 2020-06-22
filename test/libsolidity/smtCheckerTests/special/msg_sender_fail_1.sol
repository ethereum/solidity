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
// Warning 4661: (155-178): Assertion violation happens here
