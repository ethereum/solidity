contract C {
	uint z = this.g(2);

	function g(uint _x) public pure returns (uint) {
		assert(_x > 0); // should fail
		return _x;
	}

	function f() public view {
		assert(z == 2); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (87-101): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
