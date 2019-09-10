pragma experimental SMTChecker;

contract C
{
	function g() public pure {
		(uint x, uint y) = (2, 4);
		assert(x == 2);
		assert(y == 4);
	}
}
// ----
