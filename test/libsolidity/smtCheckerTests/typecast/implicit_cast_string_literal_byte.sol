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
// Warning 6328: (153-174): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
