pragma experimental SMTChecker;

contract C
{
	function f() public view {
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
