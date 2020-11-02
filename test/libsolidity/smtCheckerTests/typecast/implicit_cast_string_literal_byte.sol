pragma experimental SMTChecker;

contract C {
	mapping (byte => uint) map;
	function f() public {
		map[""] = 2;
		uint x = map[""];
		g("");
		byte b = "";
		assert(x == map[b]);
		assert(x == map["x"]);
	}
	function g(byte b) internal pure {}
}
// ----
// Warning 6328: (182-203): CHC: Assertion violation happens here.
