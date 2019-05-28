pragma experimental SMTChecker;

contract C
{
	function g() public pure {
		(uint x, ) = (2, 4);
		assert(x == 2);
	}
}
