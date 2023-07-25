type T is int224;
pragma solidity >= 0.0.0;
contract C {
	T constant public s = T.wrap(int224(165521356710917456517261742455526507355687727119203895813322792776));
	T constant public t = s;
	int224 constant public u = T.unwrap(t);

	function f() public pure {
		assert(T.unwrap(s) == 165521356710917456517261742455526507355687727119203895813322792776);
		assert(T.unwrap(t) == 165521356710917456517261742455526507355687727119203895813322792776);
		assert(u == 165521356710917456517261742455526507355687727119203895813322792776);
		assert(T.unwrap(s) == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (531-555): CHC: Assertion violation happens here.\nCounterexample:\nu = 165521356710917456517261742455526507355687727119203895813322792776\n\nTransaction trace:\nC.constructor()\nState: u = 165521356710917456517261742455526507355687727119203895813322792776\nC.f()
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
