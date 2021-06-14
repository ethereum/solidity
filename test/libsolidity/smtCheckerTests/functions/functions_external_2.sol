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
// ----
// Warning 6328: (234-253): CHC: Assertion violation happens here.
// Warning 0: (61-337): Contract invariants for :C:\n!(<errorCode> = 1)\n((!((map[1] + ((- 1) * map[0])) <= 0) || ((map'[1] + ((- 1) * map'[0])) <= 0)) && !(<errorCode> = 2) && (!((map[1] + ((- 1) * map[0])) >= 0) || ((map'[0] + ((- 1) * map'[1])) <= 0)))\n
