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
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2072: (191-207): Unused local variable.
// Warning 4588: (211-231): Assertion checker does not yet implement this type of function call.
// Warning 6328: (235-250): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 0x0\nsuccess = false\nret = []\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0x0, data) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (254-268): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0x0\nsuccess = false\nret = []\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0x0, data) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (272-291): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0x0\nsuccess = false\nret = []\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0x0, data) -- counterexample incomplete; parameter name used instead of value
// Warning 6328: (295-319): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0x0\nsuccess = false\nret = []\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0x0, data) -- counterexample incomplete; parameter name used instead of value
// Warning 3993: The BMC engine of the SMTChecker is deprecated and will be removed in a future release. Please use the CHC engine instead.
