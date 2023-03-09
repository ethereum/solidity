contract C {
	mapping (bytes16 => uint) public m;

	function f() public view {
		uint y = this.m("foo");
		assert(y == m["foo"]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (147-161): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
