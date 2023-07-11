contract State {
	function f(uint _x) public pure returns (uint) {
		assert(_x < 100); // should fail
		return _x;
	}
}
contract C {
	State s;
	uint z;

	constructor() {
		z = s.f(2);
	}

	function f() public view {
		assert(z == 2); // should hold in trusted mode
	}
}
// ====
// SMTContract: C
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Warning 6328: (69-85): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
