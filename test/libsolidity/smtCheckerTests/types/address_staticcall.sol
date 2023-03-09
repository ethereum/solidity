contract C
{
	uint x;
	mapping (uint => uint) map;
	function f(address a, bytes memory data) public {
		x = 0;
		map[0] = 0;
		mapping (uint => uint) storage localMap = map;
		(bool success, bytes memory ret) = a.staticcall(data);
		assert(success);
		assert(x == 0);
		assert(map[0] == 0);
		// Disabled because of Spacer's seg fault
		//assert(localMap[0] == 0);
	}
}
// ====
// EVMVersion: >spuriousDragon
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (127-166): Unused local variable.
// Warning 2072: (191-207): Unused local variable.
// Warning 6328: (233-248): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
