pragma experimental SMTChecker;

contract C
{
	mapping (uint => mapping (uint => mapping (uint => uint))) map;
	function f(uint x) public {
		x = 42;
		map[13][14][15] = 42;
		assert(x == map[13][14][15]);
	}
}
// ----
// Warning: (152-167): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (176-204): Assertion violation happens here
