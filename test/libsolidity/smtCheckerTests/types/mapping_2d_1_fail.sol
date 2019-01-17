pragma experimental SMTChecker;

contract C
{
	mapping (uint => mapping (uint => uint)) map;
	function f(uint x) public {
		x = 41;
		map[13][14] = 42;
		assert(x == map[13][14]);
	}
}
// ----
// Warning: (134-145): Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays.
// Warning: (154-178): Assertion violation happens here
