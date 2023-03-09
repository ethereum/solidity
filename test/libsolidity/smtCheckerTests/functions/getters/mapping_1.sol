contract C {
	mapping (uint => uint) public map;

	function f() public view {
		uint y = this.map(2);
		assert(y == map[2]); // should hold
		assert(y == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (142-156): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
