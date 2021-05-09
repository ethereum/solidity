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
		assert(map[0] == 0);
		assert(localMap[0] == 0);
	}
}
// ====
// EVMVersion: >spuriousDragon
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (191-207): Unused local variable.
// Warning 4588: (211-223): Assertion checker does not yet implement this type of function call.
// Warning 6328: (227-242): CHC: Assertion violation happens here.
// Warning 6328: (246-260): CHC: Assertion violation happens here.
// Warning 6328: (264-283): CHC: Assertion violation happens here.
// Warning 6328: (287-311): CHC: Assertion violation happens here.
// Warning 4588: (211-223): Assertion checker does not yet implement this type of function call.
