contract C
{
	uint x;
	mapping (uint => uint) map;
	function f(address a, bytes memory data) public {
		x = 0;
		map[0] = 0;
		mapping (uint => uint) storage localMap = map;
		(bool success, bytes memory ret) = a.call(data);
		assert(success);
		assert(x == 0);
		// Disabled because of Spacer nondeterminism.
		//assert(map[0] == 0);
		// Disabled because of Spacer nondeterminism.
		//assert(localMap[0] == 0);
	}
}
// ====
// EVMVersion: >spuriousDragon
// SMTEngine: all
// SMTIgnoreCex: yes
// SMTIgnoreInv: yes
// SMTIgnoreOS: macos
// ----
// Warning 2072: (127-166): Unused local variable.
// Warning 2072: (191-207): Unused local variable.
// Warning 6328: (227-242): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
