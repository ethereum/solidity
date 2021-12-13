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
		assert(map[0] == map[1]);
		assert(map[0] == 0); // should fail
	}

	function set(uint x) public {
		map[0] = x;
		map[1] = x;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreOS: macos
// ----
// Warning 6328: (234-253): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> = 1)\n((!((map[1] + ((- 1) * map[0])) >= 0) || ((map'[0] + ((- 1) * map'[1])) <= 0)) && !(<errorCode> = 2) && (!((map[1] + ((- 1) * map[0])) <= 0) || ((map'[1] + ((- 1) * map'[0])) <= 0)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(map[0] == map[1])\n<errorCode> = 2 -> Assertion failed at assert(map[0] == map[1])\n<errorCode> = 3 -> Assertion failed at assert(map[0] == 0)\n
