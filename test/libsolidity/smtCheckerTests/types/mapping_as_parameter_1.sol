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
// Warning 6328: (289-306): CHC: Assertion violation happens here.\nCounterexample:\n\na = 38\nb = 21239\n\nTransaction trace:\nc.constructor()\nc.g(38, 21239)\n    c.f(map, 38, 21239) -- internal call
