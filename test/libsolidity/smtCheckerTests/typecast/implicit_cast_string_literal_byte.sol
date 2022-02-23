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
// ====
// SMTEngine: all
// ----
// Warning 6328: (153-174): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 2\nb = 0x0\n\nTransaction trace:\nC.constructor()\nC.f()\n    C.g(0x0) -- internal call
