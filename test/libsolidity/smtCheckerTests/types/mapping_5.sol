pragma experimental SMTChecker;

contract C
{
	mapping (address => uint) map;
	function f(address a, uint x) public view {
		assert(x != map[a]);
	}
}
// ----
// Warning 6328: (125-144): Assertion violation happens here
