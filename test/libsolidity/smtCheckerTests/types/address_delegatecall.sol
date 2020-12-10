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
// Warning 2072: (224-240): Unused local variable.
// Warning 6328: (268-283): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 0\ndata = []\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0, [])
// Warning 6328: (287-301): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0\ndata = []\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0, [])
// Warning 6328: (305-324): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0\ndata = []\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0, [])
// Warning 6328: (328-352): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0\ndata = []\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nf(0, [])
