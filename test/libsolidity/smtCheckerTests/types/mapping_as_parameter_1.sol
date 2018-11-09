pragma experimental SMTChecker;

contract c {
	mapping(uint => uint) x;
	function f(mapping(uint => uint) storage map, uint index, uint value) internal {
		map[index] = value;
	}
	function g(uint a, uint b) public {
		f(x, a, b);
		// False positive since aliasing is not yet supported.
		assert(x[a] == b);
	}
}
// ----
// Warning: (289-306): Assertion violation happens here
