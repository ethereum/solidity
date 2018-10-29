pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x) public {
		map[2] = x;
		map[2] = 3;
		assert(x != map[2]);
	}
}
// ----
// Warning: (134-153): Assertion violation happens here
