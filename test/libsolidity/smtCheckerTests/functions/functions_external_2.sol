pragma experimental SMTChecker;

abstract contract D
{
	function g(uint x) public virtual;
}

contract C
{
	mapping (uint => uint) map;
	function f(uint y, D d) public {
		require(map[0] == map[1]);
		assert(map[0] == map[1]);
		d.g(y);
		// Storage knowledge is cleared after an external call.
		assert(map[0] == map[1]);
	}
}
// ----
// Warning 6328: (297-321): CHC: Assertion violation might happen here.
// Warning 4661: (297-321): BMC: Assertion violation happens here.
