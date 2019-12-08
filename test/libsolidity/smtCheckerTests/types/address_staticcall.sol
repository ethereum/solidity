pragma experimental SMTChecker;

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
		assert(localMap[0] == 0);
	}
}
// ====
// EVMVersion: >spuriousDragon
// ----
// Warning: (224-240): Unused local variable.
// Warning: (266-281): Assertion violation happens here
// Warning: (285-299): Assertion violation happens here
// Warning: (303-322): Assertion violation happens here
// Warning: (326-350): Assertion violation happens here
