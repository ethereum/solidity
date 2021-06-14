abstract contract D
{
	function g(uint x) public virtual;
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
// ====
// SMTEngine: all
// ----
// Warning 0: (61-352): Contract invariants for :C:\n!(<errorCode> >= 2)\n(<errorCode> <= 0)\n
