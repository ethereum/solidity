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
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
