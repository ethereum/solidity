contract State {
	function f(uint _x) public pure returns (uint) {
		assert(_x < 100);
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
// SMTEngine: all
// SMTExtCalls: trusted
// SMTContract: C
