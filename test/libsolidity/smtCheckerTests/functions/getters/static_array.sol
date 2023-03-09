
contract C {

	uint[2] public x = [42,1];

	function f() public view {
		assert(this.x(0) == x[0]); // should hold
		assert(this.x(1) == x[1]); // should hold
		assert(this.x(0) == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (162-184): CHC: Assertion violation happens here.
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
