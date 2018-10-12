pragma experimental SMTChecker;

contract C
{
	function f() public view {
		address a = msg.sender;
		address b = msg.sender;
		assert(a != b);
	}
}
// ----
// Warning: (128-142): Assertion violation happens here
