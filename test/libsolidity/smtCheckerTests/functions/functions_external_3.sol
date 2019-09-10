pragma experimental SMTChecker;

contract D
{
	function g(uint x) public;
}

contract C
{
	mapping (uint => uint) storageMap;
	function f(uint y, D d) public {
		mapping (uint => uint) storage map = storageMap;
		require(map[0] == map[1]);
		assert(map[0] == map[1]);
		d.g(y);
		// Storage knowledge is cleared after an external call.
		assert(map[0] == map[1]);
	}
}
// ----
// Warning: (338-362): Assertion violation happens here
