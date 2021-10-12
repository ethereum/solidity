contract D {
	uint x;
	function f() public view returns (uint) { return x; }
}

contract C {
	D d;
	constructor() {
		d = new D();
		assert(d.f() == 0); // should hold
	}
	function g() public view {
		assert(d.f() == 0); // should hold
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Info 1180: Contract invariant(s) for :C:\n((:var 1).storage.storage_D_12[d].x_3_D_12 <= 0)\nReentrancy property(ies) for :D:\n((x' <= 0) || !(x <= 0))\n
