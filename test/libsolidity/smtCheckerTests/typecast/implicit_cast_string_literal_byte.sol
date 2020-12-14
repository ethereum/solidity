pragma experimental SMTChecker;

contract C {
	mapping (bytes1 => uint) map;
	function f() public {
		map[""] = 2;
		uint x = map[""];
		g("");
		bytes1 b = "";
		assert(x == map[b]);
		assert(x == map["x"]);
	}
	function g(bytes1 b) internal pure {}
}
// ----
// Warning 6328: (186-207): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
