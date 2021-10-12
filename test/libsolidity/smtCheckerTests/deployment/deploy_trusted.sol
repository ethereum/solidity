contract D {
	uint x;
	function f() public view returns (uint) { return x; }
}

contract C {
	function g() public {
		D d = new D();
		uint y = d.f();
		assert(y == 0); // should hold in ext calls trusted mode
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
