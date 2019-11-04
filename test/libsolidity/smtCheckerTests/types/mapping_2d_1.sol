pragma experimental SMTChecker;

contract C
{
	mapping (uint => mapping (uint => uint)) map;
	function f(uint x) public {
		x = 42;
		map[13][14] = 42;
		assert(x == map[13][14]);
	}
}
// ----
