pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
