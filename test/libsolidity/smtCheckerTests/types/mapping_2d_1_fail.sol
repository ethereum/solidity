pragma experimental SMTChecker;

contract C
{
	mapping (uint => mapping (uint => uint)) map;
	function f(uint x) public view {
		x = 42;
		require(map[13][14] == 42);
		assert(x != map[13][14]);
	}
}
// ----
// Warning: (169-193): Assertion violation happens here
