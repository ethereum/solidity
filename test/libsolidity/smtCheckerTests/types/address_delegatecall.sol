pragma experimental SMTChecker;

contract C
{
	uint x;
	mapping (uint => uint) map;
	function f(address a, bytes memory data) public {
		x = 0;
		map[0] = 0;
		mapping (uint => uint) storage localMap = map;
		(bool success, bytes memory ret) = a.delegatecall(data);
		assert(success);
		assert(x == 0);
		assert(map[0] == 0);
		assert(localMap[0] == 0);
	}
}
// ====
// EVMVersion: >spuriousDragon
// ----
// Warning: (224-240): Unused local variable.
// Warning: (268-283): Assertion violation happens here
// Warning: (287-301): Assertion violation happens here
// Warning: (305-324): Assertion violation happens here
// Warning: (328-352): Assertion violation happens here
