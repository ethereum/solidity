pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) map;
	function f(uint x) public {
		map[2] = x;
		assert(x == map[2]);
	}
}
