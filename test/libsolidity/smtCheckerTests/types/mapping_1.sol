pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x) public view {
		assert(x != map[2]);
	}
}
// ----
// Warning: (111-130): Assertion violation happens here
