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
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> >= 2)\n!(<errorCode> >= 3)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(success)\n<errorCode> = 2 -> Assertion failed at assert(x == 0)\n<errorCode> = 3 -> Assertion failed at assert(map[0] == 0)\n
